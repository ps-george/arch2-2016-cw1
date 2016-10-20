/*
 * test_mips.h
 *
 *  Created on: 15 Oct 2016
 *      Author: big-g
 */

#ifndef SRC_GSP14_TEST_MIPS_H_
#define SRC_GSP14_TEST_MIPS_H_


#include "mips.h"
#include <vector>
#include <string>
/*!
 * A struct to hold pass/fail information for each
 * specific test attempted
 */
struct result_set;

/*!
 *
 * @param rfunc - String (function name)
 * @param src1 - Source 1 (register, length 5)
 * @param src2 - Source 2 (register, length 5)
 * @param dest - Destination (register, length 5)
 * @return 32 bitstream correctly formatted (big-endian) for writing to memory.
 */
uint32_t test_construct_R_bitstream(std::string rfunc, uint32_t src1, uint32_t src2, uint32_t dest);

int set_debug_level(int argc, char* argv[], mips_cpu_h cpu);

int parse_test_spec(std::string filename, std::vector<std::vector<std::string> > &spec);


/*!
 * Writes a function into memory.
 * @param mem - Memory handler (needed because don't know internals of CPU to access memory)
 * @param loc - Need to ensure PC==loc to ensure the instruction is executed.
 * @param func - String of R func to write to memory
 * @param reg1 - index of reg1
 * @param reg2 - index of reg2
 * @param shift_amt - shift amount (if shifting)
 * @param dest - index of destination register
 * @return
 */
mips_error write_R_func(
		mips_mem_h mem,
		uint32_t loc,
		std::string func,
		uint32_t reg1,
		uint32_t reg2,
		uint32_t shift_amt,
		uint32_t dest);

typedef enum _instr_type{
    instr_R_type =0,
	instr_RT_type = 0x1,
	instr_J_type = 0x2,
	instr_I_type = 0x3
}instr_type;

int mips_test_check_err(mips_error err, result_set &results);

uint32_t s_to_ui(std::string s){return (uint32_t)strtol(s.c_str(),NULL,0);}
int32_t s_to_i(std::string s){return (int32_t)strtol(s.c_str(),NULL,0);}

std::map<std::string, uint32_t> ij_to_op;

#endif /* SRC_GSP14_TEST_MIPS_H_ */
