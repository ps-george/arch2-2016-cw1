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
#include "shared.h"

using namespace std;

struct mips_cpu_impl
{
	uint32_t pc;
	uint32_t regs[32];
	mips_mem_h mem;
	unsigned debugLevel;
	FILE *debugDest;
};

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
	//msg << "regs: " << state->regs << endl;
	//msg << "mem: " << "To be completed." << endl;
	msg << "debugLevel: " << state->debugLevel << endl;
	//msg << "debugDest: " << state->debugDest << endl;
	return msg.str();
}

mips_cpu_h mips_cpu_create(mips_mem_h mem)
{
	mips_cpu_impl *state=new mips_cpu_impl;

	state->pc=0;
	for (unsigned i=0;i<32;i++){
		state->regs[i]=0;
	}

	state->debugLevel = 0;
	state->debugDest=NULL;
	state->mem=mem;

	return state;
}

/*! Reset the CPU as if it had just been created, with all registers zerod.
	However, it should not modify RAM. Imagine this as asserting the reset
	input of the CPU core.
*/
mips_error mips_cpu_reset(mips_cpu_h state){
	state->pc =0;

	for (unsigned i=0;i<32;i++){
			state->regs[i]=0;
		}

	if (state->debugLevel){
		fprintf(state->debugDest,"\nCPU state reset - new state below.\n");
		string state_str = mips_cpu_print_state(state);
		fprintf(state->debugDest,"%s\n",state_str.c_str());
	}
	//cout << "print to file location" << endl;
	return mips_Success;
}

/*! Returns the current value of one of the 32 general purpose MIPS registers */
mips_error mips_cpu_get_register(
	mips_cpu_h state,	//!< Valid (non-empty) handle to a CPU
	unsigned index,		//!< Index from 0 to 31
	uint32_t *value		//!< Where to write the value to
){
	mips_error err=mips_Success;
	*value = state->regs[index];

	// if there's an error do this: err = mips_ErrorFileReadError;

	return err;
}

/*! Modifies one of the 32 general purpose MIPS registers. */
mips_error mips_cpu_set_register(
	mips_cpu_h state,	//!< Valid (non-empty) handle to a CPU
	unsigned index,		//!< Index from 0 to 31
	uint32_t value		//!< New value to write into register file
){
	mips_error err=mips_Success;
	state->regs[index]=value;
	// if there's an error do this: err = mips_ErrorFileWriteError
	return err;
}

/*! Sets the program counter for the next instruction to the specified byte address.

	While this sets the value of the PC, it should not cause any kind of
	execution to happen. Once you look at branches in detail, you will
	see that there is some slight ambiguity about this function. Choose the
	only option that makes sense.
*/
mips_error mips_cpu_set_pc(
	mips_cpu_h state,	//!< Valid (non-empty) handle to a CPU
	uint32_t pc			//!< Address of the next instruction to exectute.
){
	mips_error err=mips_Success;
	state->pc = pc;
	// if there's an error do this: err = mips_ErrorFileWriteError
	return err;
}

/*! Gets the pc for the next instruction.

	Returns the program counter for the next instruction to be executed.
*/
mips_error mips_cpu_get_pc(
	mips_cpu_h state,	//!< Valid (non-empty) handle to a CPU
	uint32_t *pc		//!< Where to write the byte address too
){
	mips_error err=mips_Success;
	*pc = state->pc;

	return err;
}

/*! Advances the processor by one instruction.

	If an exception or error occurs, the CPU and memory state
	should be left unchanged. This is so that the user can
	inspect what happened and find out what went wrong. So
	this should be true:

	uint32_t pcOrig, pcGot;
	mips_cpu_get_pc(state, &pcOrig);
	mips_error err=mips_Success;
	err = mips_cpu_step(state);
	if(err!=mips_Success){
		mips_cpu_get_pc(state,&pcGot);
		assert(pcOrig==pcGot);
		assert(mips_cpu_step(state)==err);
	}

	Maintaining this property in all cases is actually pretty
	difficult, so _try_ to maintain it, but don't worry too
	much if under some exceptions it doesn't quite work.
*/
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
	val_l = __bswap_32(val_b);
	if (state->debugLevel){
			fprintf(state->debugDest, "%d read from memory, converted to %d.\n",val_b, val_l);
		}
	// Decode the opcode
	uint8_t opcode = val_b>>26;
	string op;
	op = opcode_to_str.at(opcode);
	// If there is an error, return err.
	if (op=="ni"){err=mips_ErrorNotImplemented;}
	else if (op=="na") {err = mips_ExceptionInvalidInstruction;}
	if (err!=mips_Success){
		return err;
	}
	// Execute the instruction...
	// If there is an error, return err.
	if (err!=mips_Success){
		return err;
	}


	return err;
}

/*! Controls printing of diagnostic and debug messages.

	You are encouraged to include diagnostic and debugging
	information in your CPU, but you should include a way
	to control how much is printed. The default should be
	to print nothing, but it is a good idea to have a way
	of turning it on and off _without_ recompiling. This function
	provides a way for the user to indicate both how much
	output they are interested in, and where that output
	should go (should it go to stdout, or stderr, or a file?).

	\param state Valid (non-empty) CPU handle.

	\param level What level of output should be printed. There
	is no specific format for the output format, the only
	requirement is that for level zero there is no output.

	\param dest Where the output should go. This should be
	remembered by the CPU simulator, then later passed
	to fprintf to provide output.

	\pre It is required that if level>0 then dest!=0, so the
	caller will always provide a valid destination if they
	have indicated they will require output.

	It is suggested that for level=1 you print out one
	line of information per instruction with basic information
	like the program counter and the instruction type, and for higher
	levels you may want to print the CPU state before each
	instruction. Both of these can usually be inserted in
	just one place in the processor, and can greatly aid
	debugging.

	However, this is completely implementation defined behaviour,
	so your simulator does not have to print anything for
	any debug level if you don't want to.

	The intent is that this function merely modifies the type
	of reporting that is performed during mips_cpu_step:

		// Enable basic debugging to stderr
		mips_cpu_set_debug_level(cpu, 1, stderr);

		// The implementation may now choose to print things
		// to stderr (what exactly is up to the implementation)
		mips_cpu_step(cpu);	// e.g. prints "pc = 0x12, encoding=R"

		// Tell the CPU to print nothing
		mips_cpu_set_debug_level(cpu, 0, NULL);

		// Now the implementation must not print any debug information
		mips_cpu_step(cpu);

		// Send detailed debug output to a text file
		FILE *dump=fopen("dump.txt", "wt");
		mips_cpu_set_debug_level(cpu, 2, dump);

		// Run lots of instructions until the CPU reports an error.
		// The implementation can write lots of useful information to
		// the file
		mips_error err=mips_Success=mips_Success;
		while(!err) {
			mips_cpu_step(cpu);
		};

		// Detach the text file, and close it
		mips_cpu_set_debug_level(cpu, 0, NULL);
		fclose(dump);

		// You can now read through the text file "dump.txt" to see what happened

	However, you could decide that you want to print something out
	at the point that mips_cpu_set_debug_level is called with level>0,
	such as the current PC and registers. Up to you.
*/
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

/*! Free all resources associated with state.

	\param state Either a handle to a valid simulation state, or an empty (NULL) handle.

	It is legal to pass an empty (NULL) handle to mips_cpu_free. It is illegal
	to pass the same non-empty handle to mips_cpu_free twice, and will
	result in undefined behaviour (i.e. anything could happen):

		mips_cpu_h cpu=mips_cpu_create(...);
		...
		mips_cpu_free(h);	// This is fine
		...
		mips_cpu_free(h);	// BANG! or nothing. Or format the hard disk.

	A better pattern is to always zero the variable after calling free,
	in case elsewhere you are not sure if resources have been released yet:

		mips_cpu_h cpu=mips_cpu_create(...);
		...
		mips_cpu_free(h);	// This is fine
		h=0;	// Make sure it is now empty
		...
		mips_cpu_free(h);	// Fine, nothing happens
		h=0;    // Does nothing here, might could stop other errors
*/
void mips_cpu_free(mips_cpu_h state){
	mips_cpu_reset(state);
	mips_mem_free(state->mem);

}
