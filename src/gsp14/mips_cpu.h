/*
 * mips_cpu.h
 *
 *  Created on: 15 Oct 2016
 *      Author: big-g
 */

#ifndef SRC_GSP14_MIPS_CPU_H_
#define SRC_GSP14_MIPS_CPU_H_

std::string mips_cpu_print_state(mips_cpu_h state);

mips_error execute_r(const uint32_t &val_l, mips_cpu_h state);
mips_error execute_rt(const uint32_t &val_l, mips_cpu_h state);
mips_error execute_i(const uint32_t &val_l,std::string op, mips_cpu_h state);
mips_error execute_j(const uint32_t &val_l,std::string op, mips_cpu_h state);

void less_than(uint32_t src1, uint32_t src2, uint32_t dest, uint32_t fn_code,mips_cpu_h state);

void add_sub_bitwise(uint32_t src1, uint32_t src2, uint32_t dest, uint32_t fn_code,mips_cpu_h state);

void mult_div(uint32_t src1,uint32_t src2,uint32_t fn_code,mips_cpu_h state);

void move_hilo(const uint32_t &fn_code, const uint32_t &dest, mips_cpu_h &state);
/*!
 * Sets PC to target and saves PC+8 in link if indicated.
 * @param target - Where to jump to.
 * @param link - If 1, save pc+8 in regs[31].
 * @param state - Used to set pc to target.
 */
mips_error jump(uint32_t target, int link, mips_cpu_h state);

void r_shift(uint32_t src1, uint32_t src2, uint32_t dest,  uint32_t shift_amt, uint32_t fn_code, mips_cpu_h state);

#endif /* SRC_GSP14_MIPS_CPU_H_ */
