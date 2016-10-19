/*
 * mips_cpu.h
 *
 *  Created on: 15 Oct 2016
 *      Author: big-g
 */

#ifndef SRC_GSP14_MIPS_CPU_H_
#define SRC_GSP14_MIPS_CPU_H_

std::string mips_cpu_print_state(mips_cpu_h state);

mips_error cpu_memory_funcs(uint32_t opcode, uint32_t s, uint32_t t,uint32_t i, mips_cpu_h state);

void cpu_decode_instr(const uint32_t &bytes,uint32_t &op,uint32_t &s,uint32_t &t,uint32_t &d,uint32_t &sh,uint32_t &fn,uint16_t &i,uint32_t &j);

mips_error cpu_execute_r(uint32_t src1, uint32_t src2, uint32_t dest, uint32_t shift_amt, uint32_t fn_code, mips_cpu_h state);
mips_error cpu_execute_rt(const uint32_t &src,const uint32_t &fn_code, const uint16_t &i, mips_cpu_h state);
mips_error cpu_execute_i(const uint32_t &s,const uint32_t &t, const uint16_t &i, const uint32_t &opcode, mips_cpu_h state);
mips_error cpu_execute_j(const uint32_t &j,const uint32_t opcode, mips_cpu_h state);

mips_error less_than(uint32_t src1, uint32_t src2, uint32_t dest, uint32_t fn_code,mips_cpu_h state);

mips_error add_sub_bitwise(uint32_t src1, uint32_t src2, uint32_t dest,uint32_t fn_code, mips_cpu_h state);

mips_error mult_div(uint32_t src1,uint32_t src2,uint32_t fn_code,mips_cpu_h state);

mips_error move_hilo(const uint32_t &fn_code, const uint32_t &dest, mips_cpu_h &state);

mips_error r_shift(uint32_t src1, uint32_t src2, uint32_t dest,  uint32_t shift_amt, uint32_t fn_code, mips_cpu_h state);
/*!
 * Sets PC to target and saves PC+8 in link if indicated.
 * @param target - Where to jump to.
 * @param link - If 1, save pc+8 in regs[31].
 * @param state - Used to set pc to target.
 */
mips_error mips_cpu_jump(uint32_t target, uint32_t link, mips_cpu_h state);


uint32_t unsigned_add(const uint32_t &a,const uint32_t &b){return a+b;}
uint32_t unsigned_sub(const uint32_t &a,const uint32_t &b){return a-b;}

#endif /* SRC_GSP14_MIPS_CPU_H_ */
