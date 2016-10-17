/*
 * mips_cpu.cpp
 *
 *  Created on: 14 Oct 2016
 *      Author: big-g
 */

#include <mips.h>
#include <string>
#include <iostream>
#include <sstream>
#include <assert.h>
#include <byteswap.h>
#include "mips_cpu.h"
#include <map>
#include <functional>

using namespace std;

const std::map<uint8_t, std::string> opcode_to_str {
	{ 0x10, "ni" },
	{ 0x11,"ni" },
	{ 0x12, "ni" },
	{ 0x13, "ni" },
	{ 0x30, "ni" },
	{ 0x31, "ni" },
	{ 0x32,"ni" },
	{ 0x33, "ni" },
	{ 0x38, "ni" },
	{ 0x39, "ni" },
	{ 0x3a,"ni" },
	{ 0x3b, "ni" },
};

struct mips_cpu_impl {
	uint32_t pc;
	uint32_t regs[32];
	mips_mem_h mem;
	unsigned debugLevel;
	FILE *debugDest;
	uint32_t hi;
	uint32_t lo;
};

mips_cpu_h mips_cpu_create(mips_mem_h mem) {
	mips_cpu_impl *state = new mips_cpu_impl;
	state->pc = 0;
	for (unsigned i = 0; i < 32; i++) {
		state->regs[i] = 0;
	}
	state->hi = 0;
	state->lo = 0;
	state->debugLevel = 0;
	state->debugDest = NULL;
	state->mem = mem;
	return state;
}

mips_error mips_cpu_reset(mips_cpu_h state) {
	state->pc = 0;

	for (unsigned i = 0; i < 32; i++) {
		state->regs[i] = 0;
	}
	state->hi = 0;
	state->lo = 0;
	if (state->debugLevel) {
		fprintf(state->debugDest, "\nCPU state reset - new state below.\n");
		string state_str = mips_cpu_print_state(state);
		fprintf(state->debugDest, "%s\n", state_str.c_str());
	}
	return mips_Success;
}

/*! Returns the current value of one of the 32 general purpose MIPS registers */
mips_error mips_cpu_get_register(mips_cpu_h state, //!< Valid (non-empty) handle to a CPU
		unsigned index,		//!< Index from 0 to 31
		uint32_t *value		//!< Where to write the value to
		) {
	mips_error err = mips_Success;

	if (!(index < 32)) {
		return mips_ErrorInvalidArgument;

	}
	*value = state->regs[index];

	return err;
}

/*! Modifies one of the 32 general purpose MIPS registers. */
mips_error mips_cpu_set_register(mips_cpu_h state,//!< Valid (non-empty) handle to a CPU
		unsigned index,		//!< Index from 0 to 31
		uint32_t value		//!< New value to write into register file
		) {
	mips_error err = mips_Success;
	if (!(index < 32)) {
		return mips_ErrorInvalidArgument;
	}
	if (index != 0) {
		state->regs[index] = value;
	}
	// if there's an error do this: err = mips_ErrorFileWriteError
	return err;
}

mips_error mips_cpu_set_pc(mips_cpu_h state,//!< Valid (non-empty) handle to a CPU
		uint32_t pc			//!< Address of the next instruction to exectute.
		) {
	mips_error err = mips_Success;
	if (pc % 4 == 0) {
		state->pc = pc;
	} else {
		err = mips_ExceptionInvalidAlignment;
	}
	return err;
}

mips_error mips_cpu_get_pc(mips_cpu_h state,//!< Valid (non-empty) handle to a CPU
		uint32_t *pc		//!< Where to write the byte address too
		) {
	mips_error err = mips_Success;
	*pc = state->pc;

	return err;
}

mips_error mips_cpu_step(mips_cpu_h state//! Valid (non-empty) handle to a CPU
		) {
	// Read the memory location defined by PC
	uint32_t val_b, val_l;
	mips_error err = mips_mem_read(state->mem, state->pc, 4, (uint8_t*) &val_b);

	// If there is an error, return err.
	if (err != mips_Success) {
		return err;
	}

	// Convert from big-endian to little-endian
	val_l = __builtin_bswap32(val_b);
	// Print converted full instruction code
	if (state->debugLevel) {
		fprintf(state->debugDest,
				"0x%08x read from memory, converted to 0x%08x.\n", val_b,
				val_l);
	}

	// Decode the opcode into constituent parts
	uint32_t s, t, d, fn, opcode, sh, j;
	uint16_t i;
	cpu_decode_instr(val_l, opcode, s, t, d, sh, fn, i, j);

	string op;
	op = opcode_to_str.at(opcode);
	if (state->debugLevel) {
		fprintf(state->debugDest, "Opcode: 0x%08x. Instruction: %s\n", opcode,
				op.c_str());
	}

	// If there is an error, return err.
	if (op == "ni") {
		return mips_ErrorNotImplemented;
	}
	// Pick whether R type, RT-type(opcode in t sub-type BLTZ etc), J-type or I-type instruction
	if (opcode == 0) {
		if (state->debugLevel) {
			fprintf(state->debugDest,
					"s(reg1): 0x%05x t(reg2):0x%05x d(dest):0x%05x sh(shift amount):x%05x fn:x%06x\n",
					s, t, d, sh, fn);
		}
		err = cpu_execute_r(s, t, d, sh, fn, state);
	}
	// RT-Type
	else if (opcode == 0x1) {
		if (state->debugLevel) {
			fprintf(state->debugDest,
					"Opcode: 0x%06x. s(reg): 0x%05x t(code):0x%05x i(imm):0x%05x\n",
					opcode, s, t, i);
		}
		err = cpu_execute_rt(s, t, i, state);
	}	// J-Type
	else if ((opcode == 0x2) || (opcode == 0x3)) {
		if (state->debugLevel) {
			fprintf(state->debugDest, "Opcode: 0x%06x. j: 0x%08x\n", opcode, j);
		}
		err = cpu_execute_j(j, opcode, state);
	} // I-Type
	else if ((opcode > 0x3) && (opcode < 0x2f)) {
		if (state->debugLevel) {
			fprintf(state->debugDest,
					"Opcode: 0x%06x. s: 0x%05x t:0x%05x i:0x%08x\n", opcode, s,
					t, i);
		}
		err = cpu_execute_i(s, t, i, opcode, state);
	} else {
		return mips_ExceptionInvalidInstruction;
	}
	return err;
}

mips_error mips_cpu_set_debug_level(mips_cpu_h state, unsigned level,
		FILE *dest) {
	mips_error err = mips_Success;
	state->debugLevel = level;
	state->debugDest = dest;
	if (level > 0) {
		fprintf(dest, "Setting debug level to %d.\n", level);
		fprintf(dest, "%s", mips_cpu_print_state(state).c_str());
	}
	return err;
}

void mips_cpu_free(mips_cpu_h state) {
	mips_cpu_reset(state);
	mips_mem_free(state->mem);
}

/*
 * FUNCTIONS
 */

mips_error cpu_memory_funcs(uint32_t opcode, uint32_t s, uint32_t t, uint32_t i,
		mips_cpu_h state) {
	mips_error err = mips_Success;
	uint16_t data_tmp;
	uint32_t val_s = state->regs[s];
	uint32_t * val_t = &(state->regs[t]);
	switch (opcode) {
	case 0x24: // LBU
		err = mips_mem_read(state->mem, (val_s + i), 1, (uint8_t*) &data_tmp);
		break;
	case 0x20: // LB
		err = mips_mem_read(state->mem, (val_s + i), 1, (uint8_t*) &data_tmp);
		data_tmp = (int8_t) data_tmp;
		break;
	case 0x21: // LH
		err = mips_mem_read(state->mem, (val_s + i), 2, (uint8_t*) &data_tmp);
		data_tmp = (int16_t) data_tmp;
		break;
	case 0x25: // LHU
		err = mips_mem_read(state->mem, (val_s + i), 2, (uint8_t*) &data_tmp);
		break;
	case 0x22: //LWL
		//! \todo Check function properly
		err = mips_mem_read(state->mem, (val_s + i), 4, (uint8_t*) &data_tmp);
		if (state->debugLevel) {
			fprintf(state->debugDest,
					"LWL read from memory, number is 0x%08x.\n", data_tmp);
		}
		*val_t = *val_t & 0x0000FFFF;
		data_tmp = *val_t | (data_tmp << 16);
		break;
	case 0x26: // LWR

		break;
	case 0x28: // SB

		break;
	case 0x29: // SH

		break;

	case 0x2b: // SW

		break;
	}
	if (err != mips_Success) {
		return err;
	}
	*val_t = data_tmp;
	state->pc += 4;
	return err;
}

string mips_cpu_print_state(mips_cpu_h state) {
	stringstream msg;
	msg << "CPU State" << endl;
	msg << "-------------" << endl;
	msg << "pc: " << state->pc << endl;
	for (unsigned i = 0; i < 32; i++) {
		msg << "Reg ";
		if (i < 10) {
			msg << " ";
		}
		msg << i << ": " << state->regs[i] << " ";
		if ((i + 1) % 4 == 0) {
			msg << endl;
		}
	}

	msg << "hi: " << state->hi << " lo: " << state->lo << endl;
	;
	//msg << "regs: " << state->regs << endl;
	//msg << "mem: " << "To be completed." << endl;
	msg << "debugLevel: " << state->debugLevel << endl;
	//msg << "debugDest: " << state->debugDest << endl;
	return msg.str();
}

mips_error mips_cpu_jump(uint32_t target, int link, mips_cpu_h state) {
	uint32_t * pc = &(state->pc);
	if (link) {
		state->regs[31] = *pc + 8;
	}
	if (target % 4 != 0) {
		return mips_ExceptionInvalidAlignment;
	}
	if (state->debugLevel) {
		fprintf(state->debugDest, "Jumping PC from %d to %d", *pc, target);
	}
	*pc = target;
	return mips_Success;
}

/*
 *Rt-Type functions!!
 */
// src = s, fn_code = t, offset = i
mips_error cpu_execute_rt(const uint32_t &src, const uint32_t &fn_code,
		const uint16_t &i, mips_cpu_h state) {
	mips_error err = mips_Success;
	const uint32_t * src_v = &((state->regs[src]));
	uint32_t * pc = &(state->pc);

	// Sign-extend the offset
	uint32_t offset = (int32_t) i;

	switch (fn_code) {
	case 0x0:	// BLTZ
		if ((int32_t) *src_v < 0) {
			return mips_cpu_jump(*pc + (offset << 2), 0, state);
		}
		break;
	case 0x1: // BGEZ
		if ((int32_t) *src_v >= 0) {
			return mips_cpu_jump(*pc + (offset << 2), 0, state);
		}
		break;
	case 0x10: // BLTZAL
		if ((int32_t) *src_v < 0) {
			return mips_cpu_jump(*pc + (offset << 2), 1, state);
		}
		break;
	case 0x11: // BGEZAL
		if ((int32_t) *src_v >= 0) {
			return mips_cpu_jump(*pc + (offset << 2), 1, state);
		}
		break;
	default:
		return mips_ErrorInvalidArgument;
		break;
	}
	*pc += 4;
	return err;
}

/*
 *J-type functions!!
 */
// target = j, opcode = opcode
mips_error cpu_execute_j(const uint32_t &j, const uint32_t opcode,
		mips_cpu_h state) {
	mips_error err = mips_Success;
	uint32_t target = j;
	target = j << 2;
	int link = 0;
	// if JAL
	if (opcode == 0x3) {
		link = 1;
	}
	err = mips_cpu_jump(target, link, state);
	return err;
}

/*
 * R-Type Functions!!!
 *
 */

void cpu_decode_instr(const uint32_t &bytes, uint32_t &op, uint32_t &s,
		uint32_t &t, uint32_t &d, uint32_t &sh, uint32_t &fn, uint16_t &i,
		uint32_t &j) {
	op = bytes >> 26;
	s = (bytes & 0x03e00000) >> 21;
	t = (bytes & 0x001f0000) >> 16;
	d = (bytes & 0x0000f800) >> 11;
	sh = (bytes & 0x000007c0) >> 6;
	fn = (bytes & 0x0000003f);
	i = (uint16_t) (bytes & 0x0000FFFF);
	j = (bytes & 0x03FFFFFF);
}

mips_error cpu_execute_r(uint32_t src1, uint32_t src2, uint32_t dest,
		uint32_t shift_amt, uint32_t fn_code, mips_cpu_h state) {
	// Decode R-type function
	mips_error err = mips_Success;
	if (state->debugLevel) {
		fprintf(state->debugDest,
				"src1 = %d, src2 = %d, dest = %d, shift_amt = %d, fn_code = 0x%02x.\n",
				src1, src2, dest, shift_amt, fn_code);
	}
	// Pass parameters and state into R-function map
	if (fn_code < 0x8) {
		r_shift(src1, src2, dest, shift_amt, fn_code, state);
	} else if (fn_code == 0x8) {
		// JR
		err = mips_cpu_jump(state->regs[src1], 0, state);
	} else if (fn_code == 0x9) {
		// JRAL
		err = mips_cpu_jump(state->regs[src1], 1, state);
	} else if ((0xf < fn_code) && (fn_code < 0x14)) {
		// MFHI, MTHI, MFLO, MTLO
		move_hilo(fn_code, dest, state);
	}

	else if ((0x17 < fn_code) && (fn_code < 0x1c)) {
		// MULT, MULTU, DIV, DIVU
		mult_div(src1, src2, fn_code, state);
	} else if ((0x1f < fn_code) && (fn_code < 0x28)) {
		// ADD, ADDU, SUB, SUBU AND,OR, XOR, NOR etc
		add_sub_bitwise(src1, src2, dest, fn_code, state);
	} else if ((0x29 < fn_code) && (fn_code < 0x2c)) {
		// SLT, SLTU
		less_than(src1, src2, dest, fn_code, state);
	} else {
		return mips_ExceptionInvalidInstruction;
	}
	return err;
}

/*
 * R-SHIFTS
 */

void print_shift_debug(
		int log, // if log = 1, logical, else arithmetic
		int drc, // If dir = 1, right, else left
		int var, // if var = 1, variable else immediate
		const uint32_t src1_v, const uint32_t src2, const uint32_t src2_v,
		const uint32_t dest, const uint32_t dest_v, const uint32_t shift_amt,
		const mips_cpu_h state) {

	const char* la = "arithmetically";
	const char* dirn = "left";
	uint32_t shift = shift_amt;
	if (log) {
		la = "logically";
	}
	if (drc) {
		dirn = "right";
	}
	if (var) {
		shift = src1_v;
	}

	if (state->debugLevel) {
		fprintf(state->debugDest,
				"Shifted value 0x%08x in reg %d by %d to the %s %s, reg %d now equals 0x%08x.\n",
				src2_v, src2, shift, dirn, la, dest, dest_v);
	}
}

void r_shift(uint32_t src1, uint32_t src2, uint32_t dest, uint32_t shift_amt,
		uint32_t fn_code, mips_cpu_h state) {
	// dest = src1 shifted left by amount shift_amt
	// If the destination == 0, we do nothing (that reg should always be 0)
	if (dest == 0) {
		state->pc += 4;
		return;
	}
	uint32_t * dest_reg = &(state->regs[dest]);
	uint32_t * src1_reg = &(state->regs[src1]);
	uint32_t * src2_reg = &(state->regs[src2]);

	switch (fn_code) {
	case 0x0: // SLL - Shift left logical (shift src2 left by shift_amt and put into dest)
		*dest_reg = *src2_reg << shift_amt;
		print_shift_debug(1, 0, 0, *src1_reg, src2, *src2_reg, dest, *dest_reg,
				shift_amt, state);
		break;
	case 0x2: // SRL - Shift right logical
		*dest_reg = *src2_reg >> shift_amt;
		print_shift_debug(1, 1, 0, *src1_reg, src2, *src2_reg, dest, *dest_reg,
				shift_amt, state);
		break;
	case 0x3: // SRA - Shift right arithmetic
		*dest_reg = ((int32_t) *src2_reg) >> shift_amt;
		print_shift_debug(0, 1, 0, *src1_reg, src2, *src2_reg, dest, *dest_reg,
				shift_amt, state);
		break;
	case 0x4: // SLLV - Shift left logical variable
		*dest_reg = *src2_reg << *src1_reg;
		print_shift_debug(1, 0, 1, *src1_reg, src2, *src2_reg, dest, *dest_reg,
				shift_amt, state);
		break;
	case 0x6: // SRLV - Shift right logical variable
		*dest_reg = *src2_reg >> *src1_reg;
		print_shift_debug(1, 1, 1, *src1_reg, src2, *src2_reg, dest, *dest_reg,
				shift_amt, state);
		break;
	case 0x7: // SRAV - Shift right arithmetic variable
		*dest_reg = ((int32_t) *src2_reg) >> *src1_reg;
		print_shift_debug(0, 1, 1, *src1_reg, src2, *src2_reg, dest, *dest_reg,
				shift_amt, state);
		break;
	}
	state->pc += 4;
	return;
}

/*
 * R-MFMT HILO
 */
void move_hilo(const uint32_t &fn_code, const uint32_t &dest,
		mips_cpu_h &state) {
	switch (fn_code) {
	case 0x10: // MFHI
		state->regs[dest] = state->hi;
		break;
	case 0x11: // MTHI
		state->hi = state->regs[dest];
		break;
	case 0x12: // MFLO
		state->regs[dest] = state->lo;
		break;
	case 0x13: // MTLO
		state->regs[dest] = state->lo;
		break;
	}
}

/*
 * R-LESS THAN
 */
void less_than(uint32_t src1, uint32_t src2, uint32_t dest, uint32_t fn_code,
		mips_cpu_h state) {
	if (dest == 0) {
		state->pc += 4;
		return;
	}
	uint32_t * dest_reg = &(state->regs[dest]);

	switch (fn_code) {
	case 0x2a: // SLT
		if ((int32_t) src1 < (int32_t) src2) {
			*dest_reg = 1;
		} else {
			*dest_reg = 0;
		}
		break;
	case 0x2b: // SLTU
		if (src1 < src2) {
			*dest_reg = 1;
		} else {
			*dest_reg = 0;
		}
		break;
	}
	state->pc += 4;
	return;
}
/*
 * R-ADD_SUB_BITWISE OPS
 */
mips_error add_sub_bitwise(uint32_t src1, uint32_t src2, uint32_t dest,
		uint32_t fn_code, mips_cpu_h state) {
	if (dest == 0) {
		state->pc += 4;
		return mips_Success;
	}
	uint32_t * dest_reg = &(state->regs[dest]);
	const uint32_t * src1_reg = &(state->regs[src1]);
	const uint32_t * src2_reg = &(state->regs[src2]);
	//! ADD, ADDU (add should have overflow catch mips_ExceptionArithmeticOverflow)
	switch (fn_code) {
	case 0x20: //ADD
		// Trap on overflow
		// return mips_ExceptionArithmeticOverflow
		//if (overflow((*src1_reg),(*src2_reg))){return mips_ExceptionArithmeticOverflow;}
		*dest_reg = (*src1_reg) + (*src2_reg);
		break;
	case 0x21:		//ADDU
		*dest_reg = (*src1_reg) + (*src2_reg);
		break;
	case 0x22: //SUB - Will trap on overflow
		*dest_reg = (*src1_reg) - (*src2_reg);
		break;
	case 0x23: //SUBU
		*dest_reg = (*src1_reg) - (*src2_reg);
		break;
	case 0x24: // AND
		*dest_reg = (*src1_reg) & (*src2_reg);
		break;
	case 0x25: // OR
		*dest_reg = (*src1_reg) | (*src2_reg);
		break;
	case 0x26: //XOR
		*dest_reg = (*src1_reg) ^ (*src2_reg);
		break;
	case 0x27: // NOR
		*dest_reg = ~((*src1_reg) | (*src2_reg));
		break;
	}
	state->pc += 4;
	return mips_Success;
}

/*
 * R_MULT DIV
 */

void mult_div(uint32_t src1, uint32_t src2, uint32_t fn_code,
		mips_cpu_h state) {
	int64_t ans;
	uint64_t u_ans;
	int32_t hi, lo;
	switch (fn_code) {
	case 0x18: // MULT
		// Cast to int first to sign extend correctly.
		ans = ((int64_t) (int32_t) src1) * ((int64_t) (int32_t) src2);
		state->hi = (uint32_t) ((ans >> 32) & 0xFFFFFFFF);
		state->lo = (uint32_t) (ans & 0xFFFFFFFF);
		break;
	case 0x19: // MULTU
		u_ans = (uint64_t) src1 * (uint64_t) src2;
		state->hi = (uint32_t) ((u_ans >> 32)) & 0xFFFFFFFF;
		state->lo = (uint32_t) (u_ans & 0xFFFFFFFF);
		break;
	case 0x1a: //DIV
		hi = ((int32_t) src1) / ((int32_t) src2);
		lo = ((int32_t) src1) % ((int32_t) src2);
		state->lo = (uint32_t) hi;
		state->hi = (uint32_t) lo;
		break;
	case 0x1b: //DIVU
		state->lo = src1 / src2;
		state->hi = src1 % src2;
	}
	state->pc += 4;
	return;
}
/*
 * END OF R-TYPE FUNCTIONS
 */

/*
 * I-TYPE FUNCTIONS
 */
mips_error cpu_execute_i(const uint32_t &s, const uint32_t &t, const uint16_t &i,
		const uint32_t &opcode, mips_cpu_h state) {
	mips_error err = mips_Success;
	const uint32_t * val_s = &(state->regs[s]);
	uint32_t * val_t = &(state->regs[t]);
	uint32_t * pc = &(state->pc);
	int32_t ss = (int32_t) state->regs[s];
	int32_t simm = (int32_t)i;
	uint32_t imm = (uint32_t)i;
	if (opcode > 0xf) {
		err = cpu_memory_funcs(opcode, s, t, i, state);
	}
	switch (opcode) {
	case 0x4: // BEQ (sign extend)
		if ((*val_s) == (*val_t)) {
			err = mips_cpu_jump((simm << 2) + state->pc, 0, state);
		}
		break;
	case 0x5: // BNE (sign extend)
		if ((*val_s) != (*val_t)) {
			err = mips_cpu_jump((simm << 2) + *pc, 0, state);
		}
		break;
	case 0x6: //BLEZ (sign extend)
		if (ss <= 0x0) {
			err = mips_cpu_jump((simm << 2) + *pc, 0, state);
			return err;
		}
		break;
	case 0x7: // BGTZ (sign extend)
		if (ss > 0x0) {
			err = mips_cpu_jump((simm << 2) + *pc, 0, state);
		}
		break;
	case 0x8: // ADDI (sign extend)
		*val_t = ss + simm;
		break;
	case 0x9: // ADDUI
		//if (overflow(ss+i)){
		//	return mips_ExceptionArithmeticOverflow;)
		//}
		*val_t = (*val_s) + imm;
		break;
	case 0xa: // SLTI (sign extend)
		if (ss < simm) {
			*val_t = 0x1;
		} else {
			*val_t = 0x0;
		}
		break;
	case 0xb: //STLIU
		if (*val_s < imm) {
			*val_t = 0x1;
		} else {
			*val_t = 0x0;
		}
		break;
	case 0xc: // ANDI (no sign extend)
		*val_t = (*val_s) & imm;
		break;
	case 0xd: //ORI
		*val_t = (*val_s) | imm;
		break;
	case 0xe: //XORI
		*val_t = (*val_s) ^ imm;
		break;
	case 0xf: //LUI
		*val_t = (imm << 16);
		break;
	}
	*pc += 4;
	return err;

}
