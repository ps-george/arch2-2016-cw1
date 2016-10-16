/*
 * test_mips.h
 *
 *  Created on: 15 Oct 2016
 *      Author: big-g
 */

#ifndef SRC_GSP14_TEST_MIPS_H_
#define SRC_GSP14_TEST_MIPS_H_


#include "mips.h"
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
uint32_t construct_R_bitstream(std::string rfunc, uint32_t src1, uint32_t src2, uint32_t dest);

/*!
 * test_add has no input params because inside of
 * test_add I will test all the possible corner cases etc.
 * that should be covered. Does not need an input.
 * @return a result set containing pass/fail
 * and details of which specific tests were failed
 * if any tests were failed.
 */
result_set test_add(mips_mem_h, mips_cpu_h);
result_set test_addi(mips_mem_h, mips_cpu_h);
result_set test_addiu(mips_mem_h, mips_cpu_h);
result_set test_addu(mips_mem_h, mips_cpu_h);
result_set test_and(mips_mem_h, mips_cpu_h);
result_set test_andi(mips_mem_h, mips_cpu_h);
result_set test_beq(mips_mem_h, mips_cpu_h);
result_set test_bgez(mips_mem_h, mips_cpu_h);
result_set test_bgezal(mips_mem_h, mips_cpu_h);
result_set test_bgtz(mips_mem_h, mips_cpu_h);
result_set test_blez(mips_mem_h, mips_cpu_h);
result_set test_bltz(mips_mem_h, mips_cpu_h);
result_set test_bltzal(mips_mem_h, mips_cpu_h);
result_set test_bne(mips_mem_h, mips_cpu_h);
result_set test_div(mips_mem_h, mips_cpu_h);
// etc...



#endif /* SRC_GSP14_TEST_MIPS_H_ */
