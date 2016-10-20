/*
 * mips_cpu.h
 *
 *  Created on: 15 Oct 2016
 *      Author: big-g
 */

#ifndef SRC_GSP14_MIPS_CPU_H_
#define SRC_GSP14_MIPS_CPU_H_
/*!
 * Performs pc = Npc.
 * Sets Npc to Jpc if delay_slot flag is up, and puts delay_slot flag down.
 * Sets Npc to pc+=4 if delay_slot flag is not up. Leaves flag down
 * @param state
 */
void mips_cpu_increment_pc(mips_cpu_h state);

/*!
 * Prints out current value of pc, all register values and the values of hi and lo.
 * @param state
 * @return
 */
std::string mips_cpu_print_state(mips_cpu_h state);

/*!
 *	Performs memory functions
 * @param opcode - Sequence of bits determines which instruction to execute based on MIPS ISA
 * @param s - Source register (bits 21-25)
 * @param t - Source register 2 (bits 16-20)
 * @param i - Immediate value (bits 0-15)
 * @param state - CPU handler
 * @return
 */
mips_error cpu_memory_funcs(uint32_t opcode, uint32_t s, uint32_t t,uint32_t i, mips_cpu_h state);

/*!
 *	Decodes bitstream read from memory and saves into referenced variables.
 * @param bytes - All 32 bytes as read from memory.
 * @param op - Output: bits 31:26
 * @param s - Output: bits 25:21
 * @param t - Output: bits 20:16
 * @param d - Output: bits 15:11
 * @param sh - Output: bits 10:6
 * @param fn - Output: bits 5:0
 * @param i - Output: bits 15:0 -> this is uint16_t, all others are uint32_t
 * @param j - Output: bits 25:0
 */
void cpu_decode_instr(const uint32_t &bytes,uint32_t &op,uint32_t &s,uint32_t &t,uint32_t &d,uint32_t &sh,uint32_t &fn,uint16_t &i,uint32_t &j);

/*!
 *
 * @param src1 - Bits 25:21
 * @param src2 - Bits 20:16
 * @param dest - Bits 15:11
 * @param shift_amt - Bits 10:6
 * @param fn_code - Bits 5:0
 * @param state - CPU handler
 * @return error code (0 = mips_Success)
 */
mips_error cpu_execute_r(uint32_t src1, uint32_t src2, uint32_t dest, uint32_t shift_amt, uint32_t fn_code, mips_cpu_h state);

/*!
 *
 * @param src - bits 25:21
 * @param fn_code - bits 20:16 (For these instructions bits 20:16 are used to encode the function)
 * @param i - bits 15:0
 * @param state - CPU handler
 * @return error code (0 = mips_Success)
 */
mips_error cpu_execute_rt(const uint32_t &src,const uint32_t &fn_code, const uint16_t &i, mips_cpu_h state);
/*!
 *
 * @param s - bits 25:21
 * @param t - bits 20:16
 * @param i - bits 15:0 (uint16_t)
 * @param opcode - bits 31:26
 * @param state - CPU handler
 * @return error code (0 = mips_Success)
 */
mips_error cpu_execute_i(const uint32_t &s,const uint32_t &t, const uint16_t &i, const uint32_t &opcode, mips_cpu_h state);
/*!
 *
 * @param j - bits 25:0
 * @param opcode - bits 31:26
 * @param state - CPU handler
 * @return error code (0 = mips_Success)
 */
mips_error cpu_execute_j(const uint32_t &j,const uint32_t opcode, mips_cpu_h state);

/*!
 *
 * @param src1 - bits 25:21
 * @param src2 - bits 20:16
 * @param dest - bits 15:11
 * @param fn_code - bits 5:0
 * @param state - CPU handler
 * @return
 */
mips_error less_than(uint32_t src1, uint32_t src2, uint32_t dest, uint32_t fn_code,mips_cpu_h state);

/*!
 *
 * @param src1 - bits 25:21
 * @param src2 - bits 20:16
 * @param dest - bits 15:11
 * @param fn_code - bits 5:0
 * @param state - CPU handler
 * @return
 */
mips_error add_sub_bitwise(uint32_t src1, uint32_t src2, uint32_t dest,uint32_t fn_code, mips_cpu_h state);

/*!
 *
 * @param src1 - bits 25:21
 * @param src2 - bits 20:16
 * @param fn_code - bits 5:0
 * @param state - CPU handler
 * @return
 */
mips_error mult_div(uint32_t src1,uint32_t src2,uint32_t fn_code,mips_cpu_h state);

/*!
 *
 * @param fn_code - bits 5:0
 * @param dest - bits 15:100
 * @param state - CPU handler
 * @return mips erro
 */
mips_error move_hilo(const uint32_t &fn_code, const uint32_t &dest, mips_cpu_h &state);

/*!
 *
 * @param src1 - bits 25:21
 * @param src2 - bits 20:16
 * @param dest - bits 15:11
 * @param shift_amt - bits 10:6
 * @param fn_code - bits 5:0
 * @param state - CPU handler
 * @return
 */
mips_error r_shift(uint32_t src1, uint32_t src2, uint32_t dest,  uint32_t shift_amt, uint32_t fn_code, mips_cpu_h state);

/*!
 * Sets PC to target and saves PC+8 in link if indicated.
 * @param target - Where to jump to.
 * @param link - If 1, save pc+8 in regs[31].
 * @param state - Used to set jPC to target.
 */
mips_error mips_cpu_jump(uint32_t target, uint32_t link, mips_cpu_h state);

#endif /* SRC_GSP14_MIPS_CPU_H_ */
