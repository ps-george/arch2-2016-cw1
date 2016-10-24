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
#include <limits.h>

using namespace std;

const std::map<uint8_t, std::string> opcode_to_str {
	{0x0,"R"},
	{0x01,"RT"},
	{0x02,"J"},
	{0x03,"JAL"},
	{0x04,"BEQ"},
	{0x05,"BNE"},
	{0x06,"BLEZ"},
	{0x07,"BGTZ"},
	{0x08,"ADDI"},
	{0x09,"ADDIU"},
	{0x0a,"SLTI"},
	{0x0b,"SLTIU"},
	{0x0c,"ANDI"},
	{0x0d,"ORI"},
	{0x0e,"XORI"},
	{0x0f,"LUI"},
	{0x20,"LB"},
	{0x21,"LH"},
	{0x22,"LWL"},
	{0x23,"LW"},
	{0x24,"LBU"},
	{0x25,"LHU"},
	{0x26,"LWR"},
	{0x28,"SB"},
	{0x29,"SH"},
	{0x2a,"SWL"},
	{0x2b,"SW"},
	{0x2e,"SWR"},
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

/*
*	include/ HEADER FILE FUNCTIONS
*/
struct mips_cpu_impl {
	uint32_t pc;
	uint32_t regs[32];
	mips_mem_h mem;
	unsigned debugLevel;
	FILE *debugDest;
	uint32_t hi;
	uint32_t lo;
	uint32_t delay_slot;
	uint32_t jPC;
	uint32_t nPC;
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
	state->delay_slot=0;
	state->nPC = 4;
	state->jPC = 0;
	return state;
}


mips_error mips_cpu_reset(mips_cpu_h state) {
	state->pc = 0;
	for (unsigned i = 0; i < 32; i++) {
		state->regs[i] = 0;
	}
	state->hi = 0;
	state->lo = 0;
	if (state->debugLevel==2) {
		fprintf(state->debugDest, "\nCPU state reset - new state below.\n");
		string state_str = mips_cpu_print_state(state);
		fprintf(state->debugDest, "%s\n", state_str.c_str());
	}
	state->delay_slot=0;
	state->nPC = 4;
	state->jPC = 0;
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
	if (pc % 4 == 0 || pc == 0) {
		state->pc = pc;
		state->nPC = pc+4;
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

mips_error mips_cpu_set_debug_level(mips_cpu_h state, unsigned level,
		FILE *dest) {
	mips_error err = mips_Success;
	state->debugLevel = level;
	state->debugDest = dest;
	if (!dest){
		state->debugDest = stderr;
	}
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
* MIPS_CPU_STEP
*/
mips_error mips_cpu_step(mips_cpu_h state//! Valid (non-empty) handle to a CPU
		) {
	// Read the memory location defined by PC
	uint32_t val_b, val_l;
	mips_error err = mips_Success;
	/*
	if (state->delay_slot){
		//! If there is a delay slot
		val_l = __builtin_bswap32(state->delay_slot);
	}
	*/

	err = mips_mem_read(state->mem, state->pc, 4, (uint8_t*) &val_b);
	if (err){
		return err;
	}
	// Convert from big-endian to little-endian
	val_l = __builtin_bswap32(val_b);

	if (state->debugLevel) {
				fprintf(state->debugDest, "\nCPU state - before step.\n");
				string state_str = mips_cpu_print_state(state);
				fprintf(state->debugDest, "%s\n", state_str.c_str());
			}

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
		/*fprintf(stdout, "Opcode: 0x%08x. Instruction: %s\n", opcode,
						op.c_str());*/
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
		/*fprintf(stdout, "Opcode: 0x%08x. Instruction: %s\n", opcode,
								op.c_str());
		*/
		return mips_ExceptionInvalidInstruction;
	}
	if (err==mips_Success || state->delay_slot){
		mips_cpu_increment_pc(state);
	}
	if (state->debugLevel) {
			fprintf(state->debugDest, "\nCPU stepped - new state below.\n");
			string state_str = mips_cpu_print_state(state);
			fprintf(state->debugDest, "%s\n", state_str.c_str());
		}
	/*
	"By convention, if an exception or interrupt prevents the completion of an
	instruction occupying a branch delay slot, the instruction stream is continued by
	re-executing the branch instruction."
	*/
	if (state->debugLevel) {
			fprintf(state->debugDest, "Error message: 0x%x\n",err);
		}
	return err;
}

/*
 * CPU General Functions
 */
 string mips_cpu_print_state(mips_cpu_h state) {
 	stringstream msg;
 	msg << "-------------" << endl;
 	msg << "pc: " << state->pc << "nPC: " << state->nPC << "jPC: " << state->jPC << endl;
 	for (unsigned i = 0; i < 32; i++) {
 		msg << "Reg ";
 		if (i < 10) {
 			msg << " ";
 		}
 		msg << dec << i << ": 0x" << hex << state->regs[i]<< dec; msg << " ";
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

int overflow(int32_t a, int32_t b){
 	// if both a and b are positive and the answer is negative, there has been overflow
 	// if both a and b are negative and the answer is positive, there has been overflow
	if ((a>0&&b>0&&(a+b<0))||(a<0&&b<0&&a+b>0)){
		return 1;
	}
	return 0;
}

/*
*	 MIPS CPU STEP(CPU) functions
*/

void mips_cpu_increment_pc(mips_cpu_h state){
 //! If there is no instruction in the delay slot, normal operation, change pc to npc
	if (state->debugLevel) {
		fprintf(state->debugDest, "PC: %d nPC: %d delay_slot: %d", state->pc, state->nPC, state->delay_slot);
	}
	state->pc=state->nPC;

	if(!state->delay_slot){
		state->nPC = (state->pc)+4;
	}
	//! If there is an instruction in the delay slot, we have just tried to execute it so
	//! remove the flag. Set pc to jPC. Set jPC to 0.
	else{
		state->delay_slot=0;
		state->nPC = state->jPC;
		state->jPC = 0;
	}
}

 void cpu_decode_instr(const uint32_t &bytes, uint32_t &op, uint32_t &s,
 		uint32_t &t, uint32_t &d, uint32_t &sh, uint32_t &fn, uint16_t &i,
 		uint32_t &j) {
 	op = bytes >> 26;
 	s = (bytes & 0x03e00000) >> 21;
 	t = (bytes & 0x001f0000) >> 16;
 	d = (bytes & 0x0000f800) >> 11;
 	sh = (bytes & 0x000007c0) >> 6;
 	fn = (bytes & 0x0000003f);
 	i = (uint16_t)(bytes & 0x0000FFFF);
 	j = (bytes & 0x03FFFFFF);
 }

 mips_error mips_cpu_jump(uint32_t target, uint32_t link, mips_cpu_h state) {
 	mips_error err = mips_Success;
 	if (link) {
 		state->regs[link] = state->pc + 8;
 	}
 	if (target % 4 != 0) {
 		return mips_ExceptionInvalidAlignment;
 	}
 	if (state->debugLevel) {
 		fprintf(state->debugDest, "Jumping PC from %d to %d", state->jPC, target);
 	}
 	state->delay_slot=1;
 	state->jPC = target;
 	return err;
 }

mips_error check_branch_negative(int32_t target, int32_t offset){
	if (target < 0 && offset<0){
		return mips_ExceptionInvalidAddress;
	}
	return mips_Success;
}
/*
 *Rt-Type functions!!
 */
// src = s, fn_code = t, offset = i

mips_error cpu_execute_rt(const uint32_t &src, const uint32_t &fn_code,
		const uint16_t &i, mips_cpu_h state) {
	mips_error err = mips_Success;
	mips_error errB = mips_Success;
	int32_t src_v = (int32_t)state->regs[src];
	uint32_t pc = state->pc;
	int32_t offset = (int16_t)i;
	int32_t target = (offset << 2) + (pc+4);
	errB = check_branch_negative(target,offset);
	switch (fn_code) {
	case 0x0:	// BLTZ
		if(state->debugLevel){fprintf(stdout,"If 0x%x is less than 0, branch to 0x%x.\n", src_v, target);}
		if (src_v < 0) {
			if (errB!=mips_Success){
				return errB;
			}
			return mips_cpu_jump(target, 0, state);
		}
		break;
	case 0x1: // BGEZ
		if(state->debugLevel){fprintf(stdout,"If 0x%x is greater than or equal to 0, branch to 0x%x.\n", src_v, target);}
		if (src_v >= 0) {
			if (errB!=mips_Success){
				return errB;
			}
			return mips_cpu_jump(target, 0, state);
		}
		break;
	case 0x10: // BLTZAL
		if (src_v < 0) {
			if (errB!=mips_Success){
				return errB;
			}
			return mips_cpu_jump(target, 31, state);
		}
		// Always set link register regardless of whether branched or not.
		state->regs[31] = pc+8;
		break;
	case 0x11: // BGEZAL
		if (src_v >= 0) {
			if (errB!=mips_Success){
				return errB;
			}
			return mips_cpu_jump(target, 31, state);
		}
		state->regs[31] = pc+8;
		break;
	default:
		return mips_ErrorInvalidArgument;
		break;
	}
	return err;
}
/*
 * END OF Rt-Type functions!!
 */
/*
 *J-type Functions
 */
mips_error cpu_execute_j(const uint32_t &j, const uint32_t opcode,
		mips_cpu_h state) {
	mips_error err = mips_Success;
	uint32_t target = j;
	target = j << 2;
	uint32_t link = 0;
	// if JAL
	if (opcode == 0x3) {
		link = 31;
	}
	err = mips_cpu_jump(target, link, state);
	return err;
}
/*
* END OF J-type Functions
*/
/*
 * I-TYPE FUNCTIONS
 */
mips_error cpu_execute_i(const uint32_t &s, const uint32_t &t, const uint16_t &i,
		const uint32_t &opcode, mips_cpu_h state) {
	mips_error err = mips_Success;
	mips_error errB = mips_Success;
	uint32_t val_s = state->regs[s];
	uint32_t * val_t = &(state->regs[t]);
	uint32_t pc = state->pc;
	int32_t sval_s = state->regs[s];
	int32_t simm = 0;
	int32_t target = 0;
	simm = (int16_t)i;
	if(state->debugLevel){fprintf(state->debugDest,
					"Simm = 0x%x\n",
					i);}
	uint32_t imm = i;
	if (opcode > 0xf) {
		err = cpu_memory_funcs(opcode, s, t, simm, state);
	}
	if (0x3<opcode&&opcode<0x8){
		target = (simm << 2) + (pc+4);
		errB = check_branch_negative(target,simm);
	}
	//! if desination reg is 0, don't do any of the following functions and just return.
	if (opcode>0x7 && t==0){
		return err;
	}
	switch (opcode) {
	case 0x4: // BEQ (sign extend)
		if ((val_s) == (*val_t)) {
			if (errB!=mips_Success){
				return errB;
			}
			err = mips_cpu_jump(target, 0, state);
		}
		break;
	case 0x5: // BNE (sign extend)
		if ((val_s) != (*val_t)) {
			if (errB!=mips_Success){
				return errB;
			}
			err = mips_cpu_jump(target, 0, state);
		}
		break;
	case 0x6: //BLEZ (sign extend)
		if (sval_s <= 0x0) {
			if (errB!=mips_Success){
				return errB;
			}
			err = mips_cpu_jump(target, 0, state);
		}
		break;
	case 0x7: // BGTZ (sign extend)
		if (sval_s > 0x0) {
			if (errB!=mips_Success){
				return errB;
			}
			err = mips_cpu_jump(target, 0, state);
		}
		break;
	case 0x8: // ADDI (sign extend)
		if (overflow(sval_s,simm)){
			return mips_ExceptionArithmeticOverflow;
		}
		*val_t = sval_s + simm;
		break;
	case 0x9: // ADDUI
		*val_t = sval_s + simm;
		break;
	case 0xa: // SLTI (sign extend)
		if (sval_s < simm) {
			*val_t = 0x1;
		} else {
			*val_t = 0x0;
		}
		break;
	case 0xb: //STLIU
		if (val_s < imm) {
			*val_t = 0x1;
		} else {
			*val_t = 0x0;
		}
		break;
	case 0xc: // ANDI (no sign extend)
		*val_t = val_s & imm;
		break;
	case 0xd: //ORI
		*val_t = val_s | imm;
		break;
	case 0xe: //XORI
		*val_t = val_s ^ imm;
		break;
	case 0xf: //LUI
		*val_t = imm << 16;
		break;
	}
	return err;
}
/*
* MIPS CPU Memory Functions
*/
mips_error cpu_memory_funcs(uint32_t opcode, uint32_t s, uint32_t t, int32_t si,
		mips_cpu_h state) {
	mips_error err = mips_Success;
	uint32_t tmp32;
	uint16_t tmp16;
	uint8_t tmp8;
	uint32_t val_s = state->regs[s];
	uint32_t * val_t = &(state->regs[t]);
	// unaligned memory load and store
	//if ((val_s + i)%4!=0){
	//	return mips_ExceptionInvalidAlignment;
	//}
	if (opcode < 0x27 && t==0){
		return err;
	}
	switch (opcode) {
	case 0x24: // LBU
		err = mips_mem_read(state->mem, (val_s + si), 1, &tmp8);
		tmp32 = (uint32_t)tmp8;
		break;
	case 0x20: // LB
		err = mips_mem_read(state->mem, (val_s + si), 1, &tmp8);
		tmp32 = (int32_t)((int8_t)tmp8);
		break;
	case 0x21: // LH
		err = mips_mem_read(state->mem, (val_s + si), 2, (uint8_t*) &tmp16);
		tmp16 = __builtin_bswap16(tmp16);
		tmp32 = (int32_t)((int16_t)tmp16);
		break;
	case 0x25: // LHU
		err = mips_mem_read(state->mem, (val_s + si), 2, (uint8_t*) &tmp16);
		tmp16 = __builtin_bswap16(tmp16);
		tmp32 = (uint32_t)tmp16;
		break;
	case 0x22: //LWL
		err = mips_mem_read(state->mem, (val_s + si), 4, (uint8_t*) &tmp32);
		if (state->debugLevel) {
			fprintf(state->debugDest,
					"LWL read from memory, number is 0x%08x.\n", tmp32);
		}
		// Load the whole word into temp and swap the bits around
		tmp32 = __builtin_bswap32(tmp32);
		// Remove the right half of the word
		tmp32 = tmp32 & 0xFFFF0000;
		*val_t = *val_t & 0x0000FFFF;
		tmp32= *val_t | tmp32;
		break;
	case 0x26: // LWR
		err = mips_mem_read(state->mem, (val_s + si), 4, (uint8_t*) &tmp32);
		if (state->debugLevel) {
			fprintf(state->debugDest,
					"LWL read from memory, number is 0x%08x.\n", tmp32);
		}
		// Load the whole word into temp and swap the bits around
		tmp32 = __builtin_bswap32(tmp32);
		// Remove the left half of the word
		tmp32 = tmp32 & 0x0000FFFF;
		*val_t = *val_t & 0xFFFF0000;
		tmp32= *val_t | tmp32;
		break;
	case 0x23: // LW
		if (((val_s + si) % 4) != 0){
			return mips_ExceptionInvalidAlignment;
		}
		err = mips_mem_read(state->mem, (val_s + si), 4, (uint8_t*) &tmp32);
		tmp32 = __builtin_bswap32(tmp32);
		break;
	case 0x28: // SB
		tmp8 = (uint8_t)(*val_t & 0x000000FF);
		err = mips_mem_write(state->mem, (val_s + si),1, &tmp8);
		break;
	case 0x29: // SH
		tmp16 = (uint16_t)(*val_t & 0x0000FFFF);
		tmp16 = __builtin_bswap16(tmp16);
		err = mips_mem_write(state->mem, (val_s + si),2, (uint8_t*)&tmp16);
		break;
	case 0x2b: // SW
		tmp32 = __builtin_bswap32(*val_t);
		if (state->debugLevel) {
			fprintf(state->debugDest,
					"Trying to store to memory location 0x%x.\n", val_s+si);
		}
		err = mips_mem_write(state->mem, (val_s+si), 4, (uint8_t*)&tmp32);
		break;
	}
	if (err != mips_Success) {
		if (state->debugLevel) {
			fprintf(state->debugDest,
					"Mips memory error 0x%x.\n", err);
		}
		return err;
	}
	*val_t = tmp32;
	return err;
}
/*
* END OF MIPS CPU Memory Functions
*/
/*
* END OF I-Type Functions
*/
/*
 * R-Type Functions
 */
mips_error cpu_execute_r(uint32_t src1, uint32_t src2, uint32_t dest,
		uint32_t shift_amt, uint32_t fn_code, mips_cpu_h state) {
	mips_error err = mips_Success;
	if (state->debugLevel) {
		fprintf(state->debugDest,
				"src1 = %d, src2 = %d, dest = %d, shift_amt = %d, fn_code = 0x%02x.\n",
				src1, src2, dest, shift_amt, fn_code);
	}
	// Pass parameters and state into specific r function handler
	if (fn_code < 0x8) {
		err = r_shift(src1, src2, dest, shift_amt, fn_code, state);
	} else if (fn_code == 0x8) {
		// JR
		err = mips_cpu_jump(state->regs[src1], 0, state);
	} else if (fn_code == 0x9) {
		// JRAL
		err = mips_cpu_jump(state->regs[src1], src2, state);
	} else if ((0xf < fn_code) && (fn_code < 0x14)) {
		// MFHI, MTHI, MFLO, MTLO
		if (shift_amt || src2){return mips_ExceptionInvalidInstruction;}
		err = move_hilo(fn_code, src1, dest, state);
	}

	else if ((0x17 < fn_code) && (fn_code < 0x1c)) {
		// MULT, MULTU, DIV, DIVU
		if (dest||shift_amt){return mips_ExceptionInvalidInstruction;}
		err= mult_div(src1, src2, fn_code, state);
	} else if ((0x1f < fn_code) && (fn_code < 0x28)) {
		// ADD, ADDU, SUB, SUBU AND,OR, XOR, NOR etc
		if (shift_amt){return mips_ExceptionInvalidInstruction;}
		err = add_sub_bitwise(src1, src2, dest, fn_code, state);

	} else if ((0x29 < fn_code) && (fn_code < 0x2c)) {
		// SLT, SLTU
		if (shift_amt){return mips_ExceptionInvalidInstruction;
		}
		err =less_than(src1, src2, dest, fn_code, state);
	} else {
		return mips_ExceptionInvalidInstruction;
	}
	return err;
}
/*
 * R-SHIFTS
 */
mips_error r_shift(uint32_t src1, uint32_t src2, uint32_t dest, uint32_t shift_amt,
		uint32_t fn_code, mips_cpu_h state) {
	// dest = src1 shifted left by amount shift_amt
	// If the destination == 0, we do nothing (that reg should always be 0)
	mips_error err = mips_Success;
	if (dest == 0) {
		return err;
	}
	uint32_t * dest_reg = &(state->regs[dest]);
	uint32_t src1_val = state->regs[src1];
	uint32_t src2_val = state->regs[src2];

	switch (fn_code) {
	case 0x0: // SLL - Shift left logical (shift src2 left by shift_amt and put into dest)
		if(src1){return mips_ExceptionInvalidInstruction;}
		*dest_reg = src2_val << shift_amt;
		print_shift_debug(1, 0, 0, src1_val, src2, src2_val, dest, *dest_reg,
				shift_amt, state);
		break;
	case 0x2: // SRL - Shift right logical
		if(src1){return mips_ExceptionInvalidInstruction;}
		*dest_reg = src2_val >> shift_amt;
		print_shift_debug(1, 1, 0, src1_val, src2, src2_val, dest, *dest_reg,
				shift_amt, state);
		break;
	case 0x3: // SRA - Shift right arithmetic
		if(src1){return mips_ExceptionInvalidInstruction;}
		*dest_reg = ((int32_t) src2_val) >> shift_amt;
		print_shift_debug(0, 1, 0, src1_val, src2, src2_val, dest, *dest_reg,
				shift_amt, state);
		break;
	case 0x4: // SLLV - Shift left logical variable
		if (shift_amt){return mips_ExceptionInvalidInstruction;}
		*dest_reg = src2_val << src1_val;
		print_shift_debug(1, 0, 1, src1_val, src2, src2_val, dest, *dest_reg,
				shift_amt, state);
		break;
	case 0x6: // SRLV - Shift right logical variable
		if (shift_amt){return mips_ExceptionInvalidInstruction;}
		*dest_reg = src2_val >> src1_val;
		print_shift_debug(1, 1, 1, src1_val, src2, src2_val, dest, *dest_reg,
				shift_amt, state);
		break;
	case 0x7: // SRAV - Shift right arithmetic variable
		if (shift_amt){return mips_ExceptionInvalidInstruction;}
		*dest_reg = ((int32_t) src2_val) >> src1_val;
		print_shift_debug(0, 1, 1, src1_val, src2, src2_val, dest, *dest_reg,
				shift_amt, state);
		break;
	}
	return err;
}
/* SHIFT Debug function
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
/*
*	END OF R-SHIFTS
*/
/*
 * R-MFMT HILO
 */
mips_error move_hilo(const uint32_t &fn_code, const uint32_t &src1,const uint32_t &dest,
		mips_cpu_h &state) {
	mips_error err = mips_Success;

	switch (fn_code) {
	case 0x10: // MFHI
		if (src1){return mips_ExceptionInvalidInstruction;}
		if (dest==0){
			return err;
		}
		state->regs[dest] = state->hi;
		break;
	case 0x11: // MTHI
		if (dest){return mips_ExceptionInvalidInstruction;}
		state->hi = state->regs[src1];
		break;
	case 0x12: // MFLO
		if (src1){return mips_ExceptionInvalidInstruction;}
		if(state->debugLevel){fprintf(state->debugDest,
						"Moving 0x%08x from lo to reg %d\n",
						state->lo, dest);}
		if (dest==0){
			return err;
		}
		state->regs[dest] = state->lo;
		break;
	case 0x13: // MTLO
		if (dest){return mips_ExceptionInvalidInstruction;}
		state->lo = state->regs[src1];
		break;
	}
	return err;
}
/*
 * END OF R-MFMT HILO
 */
/*
* R_MULT DIV
*/
mips_error mult_div(uint32_t src1, uint32_t src2, uint32_t fn_code,
 		mips_cpu_h state) {
 	mips_error err = mips_Success;
 	int64_t ans;
 	uint64_t u_ans;
 	int32_t hi, lo;
 	uint32_t val_s = state->regs[src1];
 	uint32_t val_t = state->regs[src2];
 	switch (fn_code) {
 	case 0x18: // MULT
 		// Cast to int first to sign extend correctly.
 		ans = ((int64_t)((int32_t)val_s)) * ((int64_t)((int32_t) val_t));
 		if(state->debugLevel){fprintf(state->debugDest,
 						"0x%08x * 0x%08x = 0x%lx\n",
						val_s,val_t,(uint64_t)ans);}
 		state->hi = (uint32_t) ((ans >> 32) & 0xFFFFFFFF);
 		state->lo = (uint32_t) (ans & 0xFFFFFFFF);
 		break;
 	case 0x19: // MULTU
 		u_ans = (uint64_t) val_s * (uint64_t) val_t;
 		if(state->debugLevel){fprintf(state->debugDest,
 						"0x%08x * 0x%08x = 0x%lx\n",
						val_s,val_t,u_ans);}
 		state->hi = (uint32_t) ((u_ans >> 32)) & 0xFFFFFFFF;
 		state->lo = (uint32_t) (u_ans & 0xFFFFFFFF);
 		break;
 	case 0x1a: //DIV
 		lo = ((int32_t) val_s) / ((int32_t) val_t);
 		hi = ((int32_t) val_s) % ((int32_t) val_t);
 		state->hi = (uint32_t) hi;
 		state->lo = (uint32_t) lo;
 		break;
 	case 0x1b: //DIVU
 		uint32_t div,mod;
 		div = val_s / val_t;
 		mod = val_s % val_t;
 		state->lo = val_s / val_t;
 		state->hi = val_s % val_t;
 		if(state->debugLevel){
 		fprintf(stderr,
 						"0x%x divide 0x%x = 0x%x\n0x%x modulo 0x%x = 0x%x\n",
						val_s,val_t,div,val_s,val_t,mod);
 		}
 	}
 	return err;
 }
/*
 * END OF R_MULT DIV
*/
/*
 * R-ADD_SUB_BITWISE OPS
 */
mips_error add_sub_bitwise(uint32_t src1, uint32_t src2, uint32_t dest,
		uint32_t fn_code, mips_cpu_h state) {
	mips_error err = mips_Success;
	if (dest == 0) {
		return err;
	}
	uint32_t * dest_reg = &(state->regs[dest]);
	uint32_t src1_val = state->regs[src1];
	uint32_t src2_val = state->regs[src2];
	//! ADD, ADDU (add should have overflow catch mips_ExceptionArithmeticOverflow)
	switch (fn_code) {
	case 0x20: //ADD
		if(state->debugLevel){fprintf(state->debugDest,
						"src1_val = %x, src2_val = %x",(src1_val),(src2_val));}
		if (overflow(src1_val,src2_val)){
			err = mips_ExceptionArithmeticOverflow;
			return err;
		}
		*dest_reg = (src1_val) + (src2_val);
		break;
	case 0x21:		//ADDU
		*dest_reg = (src1_val) + (src2_val);
		break;
	case 0x22: //SUB - Will trap on overflow
		if (overflow(src1_val, -src2_val)){
			err = mips_ExceptionArithmeticOverflow;
			return err;
		}
		*dest_reg = (src1_val) - (src2_val);
		break;
	case 0x23: //SUBU
		*dest_reg = (src1_val) - (src2_val);
		break;
	case 0x24: // AND
		*dest_reg = (src1_val) & (src2_val);
		break;
	case 0x25: // OR
		*dest_reg = (src1_val) | (src2_val);
		break;
	case 0x26: //XOR
		*dest_reg = (src1_val) ^ (src2_val);
		break;
	case 0x27: // NOR
		*dest_reg = ~((src1_val) | (src2_val));
		break;
	}
	return err;
}
/*
 * END OF R-ADD_SUB_BITWISE OPS
 */
/*
 * R-LESS THAN
 */
mips_error less_than(uint32_t src1, uint32_t src2, uint32_t dest, uint32_t fn_code,
		mips_cpu_h state) {
	mips_error err = mips_Success;
	if (dest == 0) {
		return err;
	}
	uint32_t * dest_reg = &(state->regs[dest]);
	uint32_t val_s = state->regs[src1];
	uint32_t val_t = state->regs[src2];
	switch (fn_code) {
	case 0x2a: // SLT
		if ((int32_t)val_s < (int32_t)val_t) {

			*dest_reg = 1;
		} else {
			state->regs[dest] = 0;
		}
		break;
	case 0x2b: // SLTU
		if(state->debugLevel){fprintf(state->debugDest,
								"%x < %x",val_s,val_t);}
		if (val_s < val_t) {
			*dest_reg = 1;
		} else {
			*dest_reg = 0;
		}
		break;
	}
	return err;
}
/*
 * ENF OF R-LESS THAN
 */
/*
 * END OF R-TYPE FUNCTIONS
 */
