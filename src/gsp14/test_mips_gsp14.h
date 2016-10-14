/*
 * test_mips.h
 *
 *  Created on: 14 Oct 2016
 *      Author: big-g
 */

#ifndef SRC_GSP14_TEST_MIPS_GSP14_
#define SRC_GSP14_TEST_MIPS_GSP14_

#endif /* SRC_GSP14_TEST_MIPS_GSP14_ */

#include "mips.h"
/*!
 * A struct to hold pass/fail information for each
 * specific test attempted
 */
struct result_set;

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



