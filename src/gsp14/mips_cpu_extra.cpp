/*
* mips_cpu_extra.cpp
 *
 *  Created on: 15 Oct 2016
 *      Author: big-g
 */


/*
mips_error r_SLL(int,int,int,int, mips_cpu_h);
mips_error r_SRL(int,int,int,int, mips_cpu_h);
mips_error r_SRA(int,int,int,int, mips_cpu_h);
mips_error r_SLLV(int,int,int,int, mips_cpu_h);
mips_error r_JR(int,int,int,int, mips_cpu_h);
mips_error r_JALR(int,int,int,int, mips_cpu_h);
mips_error r_MFHI(int,int,int,int, mips_cpu_h);
mips_error r_MTHI(int,int,int,int, mips_cpu_h);
mips_error r_MFLO(int,int,int,int, mips_cpu_h);
mips_error r_MTLO(int,int,int,int, mips_cpu_h);
mips_error r_MULT(int,int,int,int, mips_cpu_h);
mips_error r_MULTU(int,int,int,int, mips_cpu_h);
mips_error r_DIV(int,int,int,int, mips_cpu_h);
mips_error r_DIVU(int,int,int,int, mips_cpu_h);
mips_error r_MTHI(int,int,int,int, mips_cpu_h);
mips_error r_ADD(int,int,int,int, mips_cpu_h);
mips_error r_ADDU(int,int,int,int, mips_cpu_h);
mips_error r_SUB(int,int,int,int, mips_cpu_h);
mips_error r_SUBU(int,int,int,int, mips_cpu_h);
mips_error r_AND(int,int,int,int, mips_cpu_h);
mips_error r_OR(int,int,int,int, mips_cpu_h);
mips_error r_XOR(int,int,int,int, mips_cpu_h);
mips_error r_NOR(int,int,int,int, mips_cpu_h);
mips_error r_SLT(int,int,int,int, mips_cpu_h);
mips_error r_SLTU(int,int,int,int, mips_cpu_h);
*/

/*
const std::map<uint32_t, function<mips_error(uint32_t,uint32_t,uint32_t,uint32_t, mips_cpu_h)>> rop_to_rfunc{
	{0x0, r_shift},		// "SLL"
	{0x01, r_na}, 		// "na"
	{0x02, r_shift}, 	//"SRL"
	{0x03,r_shift}, 	//"SRA"
	{0x04,r_shift}, 	//"SLLV"
	{0x05, r_na}, 		//"na"
	{0x06,r_shift}, 	//"SRLV"
	{0x07,r_shift}, 	//"SRAV"
	{"JR",0x08},
	{"JALR",0x09},
	{"na",0x0a},
	{"na",0x0b},
	{"ni",0x0c},
	{"ni",0x0d},
	{"na",0x0e},
	{"na",0x0f},
	{"MFHI",0x10},
	{"MTHI",0x11},
	{"MFLO",0x12},
	{"MTLO",0x13},
	{"na",0x14},
	{"na",0x15},
	{"na",0x16},
	{"na",0x17},
	{"MULT",0x18},
	{"MULTU",0x19},
	{"DIV",0x1a},
	{"DIVU",0x1b},
	{"na",0x1c},
	{"na",0x1d},
	{"na",0x1e},
	{"na",0x1f},
	{"ADD",0x20},
	{"ADDU",0x21},
	{"SUB",0x22},
	{"SUBU",0x23},
	{"AND",0x24},
	{"OR",0x25},
	{"XOR",0x26},
	{"NOR",0x27},
	{"na",0x28},
	{"na",0x29},
	{"SLT",0x2a},
	{"SLTU",0x2b}
};*/
