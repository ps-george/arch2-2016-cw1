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

int parse_test_spec(std::string filename, std::vector<std::vector<std::string> > &spec);

int set_debug_level(int argc, char* argv[], mips_cpu_h cpu);

uint32_t test_construct_bitstream(std::string func,const std::vector<uint32_t> &params);

void compare_model(mips_cpu_h cpu, model_state model, result_set &results);

int mips_test_check_err(mips_error err, result_set &results);

void run_spec(const std::vector<std::vector<std::string>> &spec, mips_mem_h mem, mips_cpu_h cpu);

void test_r_type(const std::vector<std::string> &row,result_set &results, mips_mem_h mem, mips_cpu_h cpu);

void test_rt_type(const std::vector<std::string> &row, result_set &results, mips_mem_h mem, mips_cpu_h cpu);

void test_j_type(const std::vector<std::string> &row, result_set &results, mips_mem_h mem, mips_cpu_h cpu);

void test_i_type(const std::vector<std::string> &row, result_set &results, mips_mem_h mem, mips_cpu_h cpu);

std::map<std::string, uint32_t> ij_to_op;

#endif /* SRC_GSP14_TEST_MIPS_H_ */
