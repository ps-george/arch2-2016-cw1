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

const std::map<uint8_t, std::string> opcode_to_str{
	{0x00,"R"},{0x01,"BCND"},{0x02,"J"},{0x03,"JAL"},
	{0x04,"BEQ"}, //! \todo
	{0x05,"BNE"}, //! \todo
	{0x06,"BLEZ"}, //! \todo
	{0x07,"BGTZ"}, //! \todo
	{0x08,"ADDI"}, //! \todo
	{0x09,"ADDIU"}, //! \todo
	{0x0a,"SLTI"}, //! \todo
	{0x0b,"SLTIU"}, //! \todo
	{0x0c,"ANDI"},//! \todo
	{0x0d,"ORI"}, //! \todo
	{0x0e,"XORI"}, //! \todo
	{0x0f,"LUI"}, //! \todo
	{0x10,"ni"},{0x11,"ni"},{0x12,"ni"},{0x13,"ni"},
	{0x14,"nv"},{0x15,"nv"},{0x16,"nv"},{0x17,"nv"},
	{0x18,"nv"},{0x19,"nv"},{0x1a,"nv"},{0x1b,"nv"},
	{0x1c,"nv"},{0x1d,"nv"},{0x1e,"nv"},{0x1f,"nv"},
	{0x20,"LB"}, //! \todo
	{0x21,"LH"}, //! \todo
	{0x22,"LWL"}, //! \todo
	{0x23,"LW"}, //! \todo
	{0x24,"LBU"}, //! \todo
	{0x25,"LHU"}, //! \todo
	{0x26,"LWR"}, //! \todo
	{0x27,"nv"},
	{0x28,"SB"}, //! \todo
	{0x29,"SH"}, //! \todo
	{0x2a,"SWL"},
	{0x2b,"SW"}, //! \todo 26
	{0x2c,"nv"},{0x2d,"nv"},
	{0x2e,"SWR"}, //!
	{0x2f,"nv"},{0x30,"ni"},{0x31,"ni"},{0x32,"ni"},
	{0x33,"ni"},{0x34,"nv"},{0x35,"nv"},{0x36,"nv"},
	{0x37,"nv"},{0x38,"ni"},{0x39,"ni"},{0x3a,"ni"},
	{0x3b,"ni"},{0x3c,"nv"},{0x3d,"nv"},{0x3e,"nv"},{0x3f,"nv"}
};

struct mips_cpu_impl
{
	uint32_t pc;
	uint32_t regs[32];
	mips_mem_h mem;
	unsigned debugLevel;
	FILE *debugDest;
	uint32_t hi;
	uint32_t lo;
};

mips_cpu_h mips_cpu_create(mips_mem_h mem)
{
	mips_cpu_impl *state=new mips_cpu_impl;
	state->pc=0;
	for (unsigned i=0;i<32;i++){
		state->regs[i]=0;
	}
	state->hi=0;
	state->lo=0;
	state->debugLevel = 0;
	state->debugDest=NULL;
	state->mem=mem;
	return state;
}

mips_error mips_cpu_reset(mips_cpu_h state){
	state->pc =0;

	for (unsigned i=0;i<32;i++){
			state->regs[i]=0;
		}
	state->hi=0;
	state->lo=0;
	if (state->debugLevel){
		fprintf(state->debugDest,"\nCPU state reset - new state below.\n");
		string state_str = mips_cpu_print_state(state);
		fprintf(state->debugDest,"%s\n",state_str.c_str());
	}
	return mips_Success;
}

/*! Returns the current value of one of the 32 general purpose MIPS registers */
mips_error mips_cpu_get_register(
	mips_cpu_h state,	//!< Valid (non-empty) handle to a CPU
	unsigned index,		//!< Index from 0 to 31
	uint32_t *value		//!< Where to write the value to
){
	mips_error err=mips_Success;

	if (index<32){
		*value = state->regs[index];
	}
	else{err=mips_ErrorInvalidArgument;}

	return err;
}

/*! Modifies one of the 32 general purpose MIPS registers. */
mips_error mips_cpu_set_register(
	mips_cpu_h state,	//!< Valid (non-empty) handle to a CPU
	unsigned index,		//!< Index from 0 to 31
	uint32_t value		//!< New value to write into register file
){
	mips_error err=mips_Success;
	if (index<32){
		state->regs[index]=value;
	}
	else{err=mips_ErrorInvalidArgument;}
		// if there's an error do this: err = mips_ErrorFileWriteError
	return err;
}

mips_error mips_cpu_set_pc(
	mips_cpu_h state,	//!< Valid (non-empty) handle to a CPU
	uint32_t pc			//!< Address of the next instruction to exectute.
){
	mips_error err=mips_Success;
	if (pc%4==0){
		state->pc = pc;
	}
	else{err=mips_ExceptionInvalidAlignment;}
	return err;
}

mips_error mips_cpu_get_pc(
	mips_cpu_h state,	//!< Valid (non-empty) handle to a CPU
	uint32_t *pc		//!< Where to write the byte address too
){
	mips_error err=mips_Success;
	*pc = state->pc;

	return err;
}

mips_error mips_cpu_step(
	mips_cpu_h state	//! Valid (non-empty) handle to a CPU
){
	// Read the memory location defined by PC
	uint32_t val_b, val_l;
	mips_error err = mips_mem_read(state->mem, state->pc,4,(uint8_t*)&val_b);

	// If there is an error, return err.
	if (err!=mips_Success){
		return err;
	}

	// Convert from big-endian to little-endian
	val_l = __builtin_bswap32(val_b);
	if (state->debugLevel){
			fprintf(state->debugDest, "0x%08x read from memory, converted to 0x%08x.\n",val_b, val_l);
		}

	// Decode the opcode
	uint8_t opcode = val_l>>26;
	string op;
	op = opcode_to_str.at(opcode);

	// If there is an error, return err.
	if (op=="ni"){return mips_ErrorNotImplemented;}
	else if (op=="nv") {return mips_ExceptionInvalidInstruction;}
	// Execute the instruction!
	else if (opcode==0x0){
		err = execute_r(val_l, state);
	}
	else if (opcode==0x1) {
		err = execute_rt(val_l,state);
	}
	else if ((opcode==0x2)||(opcode==0x3)){
		err = execute_j(val_l,op,state);
	}
	else if (opcode>0x3){
		err = execute_i(val_l,op,state);
	}
	return err;
}

mips_error mips_cpu_set_debug_level(mips_cpu_h state, unsigned level, FILE *dest){
	mips_error err=mips_Success;
	state->debugLevel = level;
	state->debugDest = dest;
	//! \todo Don't understand how to use FILE object to print to file or stderr
	if (level > 0 ) {
		fprintf(dest, "Setting debug level to %d.\n", level);
		fprintf(dest, "%s",mips_cpu_print_state(state).c_str());
	}
	return err;
}

void mips_cpu_free(mips_cpu_h state){
	mips_cpu_reset(state);
	mips_mem_free(state->mem);
}

mips_error execute_i(const uint32_t &val_l,std::string op, mips_cpu_h state){
	mips_error err = mips_Success;



	return err;

}

/*
 * FUNCTIONS
 */

string mips_cpu_print_state(mips_cpu_h state){
	stringstream msg;
	msg << "CPU State" << endl;
	msg << "-------------" << endl;
	msg << "pc: " << state->pc << endl;
	for (unsigned i=0;i<32;i++){
		msg << "Reg ";
		if (i<10){
			msg << " ";
		}
		msg << i << ": " << state->regs[i] << " ";
		if ((i+1)%4==0){
			msg << endl;
		}
		}

	msg << "hi: " << state->hi << " lo: " << state->lo << endl;;
	//msg << "regs: " << state->regs << endl;
	//msg << "mem: " << "To be completed." << endl;
	msg << "debugLevel: " << state->debugLevel << endl;
	//msg << "debugDest: " << state->debugDest << endl;
	return msg.str();
}

uint32_t unsigned_add(const uint32_t &a,const uint32_t &b){return a+b;}
uint32_t unsigned_sub(const uint32_t &a,const uint32_t &b){return a-b;}
int32_t signed_add(const int32_t &a,const int32_t &b){return a+b;}
int32_t signed_sub(const int32_t &a,const int32_t &b){return a-b;}


mips_error jump(uint32_t target, int link, mips_cpu_h state){
	uint32_t * pc = &(state->pc);
	if(link){state->regs[31] = *pc + 8;}
	if (target%4!=0){
		return mips_ExceptionInvalidAlignment;
	}
	if (state->debugLevel){
		fprintf(state->debugDest,
				"Jumping PC from %d to %d",
				*pc,target);
	}
	*pc = target;
	return mips_Success;
}


/*
 *Rt-Type functions!!
 */

mips_error execute_rt(const uint32_t &val_l, mips_cpu_h state){
	mips_error err = mips_Success;
	uint32_t offset,fn_code,src;
	fn_code = (val_l & 0x001f0000)>>16;
	src = (val_l & 0x03e00000)>>21;
	offset = (val_l & 0x0000FFFF);
	const uint32_t * src_v = &((state->regs[src]));
	uint32_t * pc = &(state->pc);
	if (fn_code ==0x0){
		//BLTZ
		if ((int32_t)*src_v<0){
			err = jump(*pc+(offset<<2),0,state);
		}
		else{*pc+=4;}
	}
	else if (fn_code==0x1){
		//BGEZ
		if ((int32_t)*src_v>=0){
			err = jump(*pc+(offset<<2),0,state);
		}
		else{*pc+=4;}
	}
	else if (fn_code==0x10){
		// BLTZAL
		if ((int32_t)*src_v<0){
			err = jump(*pc+(offset<<2),1,state);
		}
		else{*pc+=4;}
	}
	else if (fn_code==0x11){
		//BGEZAL
		if((int32_t)*src_v>=0){
			err = jump(*pc+(offset<<2),1,state);
		}
	}
	else {err = mips_ErrorInvalidArgument;}

	return err;
}

/*
 *J-type functions!!
 */

mips_error execute_j(const uint32_t &val_l,string op, mips_cpu_h state){
	mips_error err = mips_Success;
	uint32_t target = 0;
	target = (val_l & 0x03FFFFFF)<<2;
	int link = 0;
	if (op=="JAL"){link = 1;}
	err = jump(target,link,state);
	return err;
}

/*
 * R-Type Functions!!!
 *
 */

void decode_r_type(const uint32_t &bytes, uint32_t &src1,uint32_t &src2,uint32_t &dest,uint32_t &shift_amt, uint32_t &fn_code){
	fn_code = (bytes & 0x0000003f);
	shift_amt = (bytes & 0x000007c0)>>6;
	dest = (bytes & 0x0000f800)>>11;
	src2 = (bytes & 0x001f0000)>>16;
	src1 = (bytes & 0x03e00000)>>21;
}

mips_error execute_r(const uint32_t &val_l, mips_cpu_h state){
	// Decode R-type function
	uint32_t src1, src2, dest, shift_amt, fn_code;
	decode_r_type(val_l,src1, src2, dest, shift_amt, fn_code);
	mips_error err = mips_Success;
	if (state->debugLevel){
		fprintf(state->debugDest, "src1 = %d, src2 = %d, dest = %d, shift_amt = %d, fn_code = 0x%02x.\n",src1,src2,dest,shift_amt,fn_code);
	}
	// Pass parameters and state into R-function map
	if (fn_code<0x8){
		r_shift(src1, src2, dest, shift_amt, fn_code, state);
	}
	else if (fn_code==0x8){
		// JR
		err = jump(state->regs[src1],0,state);
	}
	else if (fn_code==0x9){
		// JRAL
		err = jump(state->regs[src1],1,state);
	}
	else if ((0xf<fn_code)&&(fn_code<0x14)){
		// MFHI, MTHI, MFLO, MTLO
		move_hilo(fn_code, dest, state);
	}

	else if ((0x17<fn_code)&&(fn_code<0x1c)){
		// MULT, MULTU, DIV, DIVU
		mult_div(src1, src2, fn_code, state);
	}
	else if ((0x1f<fn_code)&&(fn_code<0x28)){
		// ADD, ADDU, SUB, SUBU AND,OR, XOR, NOR etc
		add_sub_bitwise(src1, src2, dest, fn_code, state);
	}
	else if ((0x29<fn_code)&&(fn_code<0x2c)){
		// SLT, SLTU
		less_than(src1,src2,dest,fn_code,state);
	}
	else{return mips_ExceptionInvalidInstruction;}
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
		const uint32_t dest,const uint32_t dest_v,
		const uint32_t shift_amt,
		const mips_cpu_h state){

	const char* la = "arithmetically";
	const char* dirn = "left";
	uint32_t shift = shift_amt;
	if (log){la = "logically";}
	if (drc){dirn = "right";}
	if (var){shift = src1_v;}

	if (state->debugLevel){
		fprintf(state->debugDest,
				"Shifted value 0x%08x in reg %d by %d to the %s %s, reg %d now equals 0x%08x.\n",
				src2_v,src2,shift,dirn,la,dest, dest_v);
	}
}

void r_shift(uint32_t src1, uint32_t src2, uint32_t dest,  uint32_t shift_amt, uint32_t fn_code, mips_cpu_h state){
	// dest = src1 shifted left by amount shift_amt
	// If the destination == 0, we do nothing (that reg should always be 0)
	if (dest == 0){
		state->pc += 4;
		return;
	}
	uint32_t * dest_reg = &(state->regs[dest]);
	uint32_t * src1_reg = &(state->regs[src1]);
	uint32_t * src2_reg = &(state->regs[src2]);
	// SLL - Shift left logical (shift src2 left by shift_amt and put into dest)
	if (fn_code == 0x0){
		*dest_reg = *src2_reg<<shift_amt;
		print_shift_debug(1,0,0,*src1_reg, src2, *src2_reg, dest, *dest_reg,shift_amt, state);
	}
	// SRL - Shift right logical
	if (fn_code == 0x2){
		*dest_reg = *src2_reg>>shift_amt;
		print_shift_debug(1,1,0,*src1_reg, src2, *src2_reg, dest, *dest_reg,shift_amt, state);
	}
	// SRA - Shift right arithmetic
	if (fn_code == 0x3){
		*dest_reg = ((int32_t)*src2_reg)>>shift_amt;
		print_shift_debug(0,1,0,*src1_reg, src2, *src2_reg, dest, *dest_reg,shift_amt, state);
	}
	// SLLV - Shift left logical variable
	if (fn_code == 0x4){
		*dest_reg = *src2_reg<<*src1_reg;
		print_shift_debug(1,0,1,*src1_reg, src2, *src2_reg, dest, *dest_reg,shift_amt, state);
	}
	// SRLV
	if (fn_code == 0x6){
		*dest_reg = *src2_reg>>*src1_reg;
		print_shift_debug(1,1,1,*src1_reg, src2, *src2_reg, dest, *dest_reg,shift_amt, state);
	}
	//SRAV
	if (fn_code == 0x7){
		*dest_reg = ((int32_t)*src2_reg)>>*src1_reg;
		print_shift_debug(0,1,1,*src1_reg, src2, *src2_reg, dest, *dest_reg,shift_amt, state);
	}
	state->pc += 4;
	return;
}

/*
 * R-MFMT HILO
 */
void move_hilo(const uint32_t &fn_code, const uint32_t &dest, mips_cpu_h &state){
	if (fn_code==0x10){
		state->regs[dest] = state->hi;
	}
	else if (fn_code==0x11){
		state->hi = state->regs[dest];
	}
	else if (fn_code==0x12){
		state->regs[dest] = state->lo;
	}
	else if (fn_code==0x12){
		state->regs[dest] = state->lo;
	}
}

/*
 * R-LESS THAN
 */
void less_than(uint32_t src1, uint32_t src2, uint32_t dest, uint32_t fn_code,mips_cpu_h state){
	if (dest==0){
		state->pc+=4;
		return;
	}
	uint32_t * dest_reg = &(state->regs[dest]);
	if (fn_code==0x2a){
		if ((int32_t)src1<(int32_t)src2){*dest_reg = 1;}
		else{*dest_reg=0;}
	}
	else if (fn_code==0x2b){
		if (src1<src2){*dest_reg=0;}
		else{*dest_reg=1;}
	}
	state->pc+=4;
	return;
}
/*
 * R-ADD_SUB_BITWISE OPS
 */
void add_sub_bitwise(uint32_t src1, uint32_t src2, uint32_t dest, uint32_t fn_code,mips_cpu_h state){
	if (dest==0){
		state->pc+=4;
		return;
	}
	uint32_t * dest_reg = &(state->regs[dest]);
	const uint32_t * src1_reg = &(state->regs[src1]);
	const uint32_t * src2_reg = &(state->regs[src2]);
	if (fn_code==0x20){
		*dest_reg = signed_add(*src1_reg, *src2_reg);
	}
	if (fn_code==0x21){
		*dest_reg = unsigned_add(*src1_reg,*src2_reg);
	}
	if (fn_code==0x22){
		*dest_reg = signed_sub(*src1_reg, *src2_reg);
	}
	if (fn_code==0x23){
		*dest_reg = unsigned_sub(*src1_reg, *src2_reg);
	}
	if (fn_code==0x24){
		*dest_reg = (*src1_reg)&(*src2_reg);
	}
	if (fn_code==0x25){
		*dest_reg = (*src1_reg)|(*src2_reg);
	}
	if (fn_code==0x26){
		*dest_reg = ~(*src1_reg)|(*src2_reg);
	}
	if (fn_code==0x27){
		*dest_reg = (*src1_reg)^(*src2_reg);
	}
	state->pc+=4;
	return;
}

/*
 * R_MULT DIV
 */

void mult_div(uint32_t src1,uint32_t src2,uint32_t fn_code,mips_cpu_h state){
	int64_t ans;
	uint64_t uans;
	int32_t hi32, lo32;
	uint32_t uhi32, ulo32;
	// MULT
	if (fn_code == 0x18){
		ans = ((int64_t)src1)*((int64_t)src2);
		hi32 = (ans)>>32;
		lo32 = (ans & 0xFFFFFFFF);
		state->hi = hi32;
		state->lo = lo32;
	}
	//MULTU
	if (fn_code == 0x19){
		uans = (uint64_t)src1*(uint64_t)src2;
		uhi32 = (uans)>>32;
		ulo32 = (uans & 0xFFFFFFFF);
		state->hi = uhi32;
		state->lo = ulo32;
	}
	// DIV
	if (fn_code == 0x1a){
		hi32 = ((int32_t)src1)/((int32_t)src2);
		lo32 = ((int32_t)src1)%((int32_t)src2);
		state->lo = hi32;
		state->hi = lo32;
	}
	// DIVU
	if (fn_code == 0x1b){
		state->lo = src1/src2;
		state->hi = src1%src2;
	}
	state->pc+=4;
	return;
}
/*
 * END OF R-TYPE FUNCTIONS
 */
