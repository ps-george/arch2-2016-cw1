/*
 * mips_cpu.cpp
 *
 *  Created on: 14 Oct 2016
 *      Author: big-g
 */

#include <mips.h>

struct mips_cpu_impl
{
	uint32_t pc;
	uint32_t regs[32];
	mips_mem_h mem;
};

mips_cpu_h mips_cpu_create(mips_mem_h mem)
{
	mips_cpu_impl *state=new mips_cpu_impl;

	state->pc=0;
	for (unsigned i=0;i<32;i++){
		state->regs[i]=0;
	}
	state->mem=mem;

	return state;
}

