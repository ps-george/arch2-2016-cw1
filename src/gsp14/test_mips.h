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
struct model_state;

typedef enum _instr_type{
  instr_R_type =0,
	instr_RT_type = 0x1,
	instr_J_type = 0x2,
	instr_I_type = 0x3
}instr_type;

uint32_t s_to_ui(std::string s){return (uint32_t)strtol(s.c_str(),NULL,0);}
int32_t s_to_i(std::string s){return (int32_t)strtol(s.c_str(),NULL,0);}

/*!
 * Parses the filename specified, and saves each parameter in a row into a string vector,
 * saving each entire row into an over-arching vector (2D array).
 * @param filename - expecting a .csv
 * @param spec - Output variable passed by reference (we were taught last year to return vector arguments by reference).
 * @return - Successful return 0, else return 1.
 */
int parse_test_spec(std::string filename, std::vector<std::vector<std::string> > &spec);

/*!
 * Checks the arguments given when executing the program to set the debug level of the CPU.
 * Checks for -d in argv[2], if it is there, argv[3] is the level to set the debug level to.
 * Checks for -f in argv[4], if it is there, argv[5] is the file to save the debug output to.
 * If -d is there but -f is not, sets debug destination to stdout.
 * @param argc - Number of arguments passed in execution of the program
 * @param argv - Array containing arguments passed to program
 * @param cpu - cpu handler, used to set debug level.
 * @return
 */
int set_debug_level(int argc, char* argv[], mips_cpu_h cpu);

/*!
 * Constructs a bitstream to be written to memory depending on the contents of params.
 * \todo Maybe instead of checking first item of params vector, should take another input
 * (Separation of concerns) - params is parameters, uint8_t type is the type.
 * Sounds like a good idea, may do that later.
 * @param func - Name of the function e.g. "ADD", or "ADDIU"
 * @param params - Vector of uint32_ts, passed by reference to avoid unnecessary copying. First item determines instruction type.
 * @return
 */
uint32_t test_construct_bitstream(std::string func,const std::vector<uint32_t> &params);

void compare_model(mips_cpu_h cpu, model_state model, result_set &results);

int mips_test_check_err(mips_error err, result_set &results);

/*!
 *
 * @param spec
 * @param mem
 * @param cpu
 */
void run_spec(const std::vector<std::vector<std::string>> &spec, mips_mem_h mem, mips_cpu_h cpu);

/*!
 * Separating tests by instructions is a bad idea because there are lots of different test types
 * which are common to different instruction types. e.g. the ways you test a branch instruction
 * are similar to how you test a jump instruction, and the ways you test normal add, addi instructions
 * are very similar.
 * 17 instructions implemented in .csv files and working in test_r_type, will be replaced by test_normal_ etc.
 * @param row
 * @param results
 * @param mem
 * @param cpu
 */
void test_r_type(const std::vector<std::string> &row,result_set &results, mips_mem_h mem, mips_cpu_h cpu);

/*!
 * Not implemented yet, will be replaced by test_normal_ etc.
 * @param row
 * @param results
 * @param mem
 * @param cpu
 */
void test_rt_type(const std::vector<std::string> &row, result_set &results, mips_mem_h mem, mips_cpu_h cpu);

/*!
 * Not implemented yet, will be replaced by test_normal_ etc.
 * @param row
 * @param results
 * @param mem
 * @param cpu
 */
void test_j_type(const std::vector<std::string> &row, result_set &results, mips_mem_h mem, mips_cpu_h cpu);

/*!
 * Not implemented yet, will be replaced by test_normal_ etc.
 * @param row
 * @param results
 * @param mem
 * @param cpu
 */
void test_i_type(const std::vector<std::string> &row, result_set &results, mips_mem_h mem, mips_cpu_h cpu);

/*!
 * \todo Not implemented yet.
 * @param row
 * @param results
 * @param mem
 * @param cpu
 */
void test_normal_functions(const std::vector<std::string> &row, result_set &results, mips_mem_h mem, mips_cpu_h cpu);

std::map<std::string, uint32_t> ij_to_op;

#endif /* SRC_GSP14_TEST_MIPS_H_ */
