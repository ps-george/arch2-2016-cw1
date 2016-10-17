/*
 * test_mips.cpp
 *
 *  Created on: 14 Oct 2016
 *      Author: big-g
 */

#include "string.h"
#include <iostream>
#include <sstream>
#include <functional>
#include <map>
#include <string>
#include "test_mips.h"
#include <iomanip>

const std::map<std::string, uint8_t> r_to_op{
	{"SLL",0x0},
	{"na",0x01},
	{"SRL",0x02},
	{"SRA",0x03},
	{"SLLV",0x04},
	{"na",0x05},
	{"SRLV",0x06},
	{"SRAV",0x07},
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
};

const std::map<uint8_t, std::string> op_to_str{
	{0x00,"R"}, //! \todo
	{0x01,"BCND"}, //! \todo
	{0x02,"J"}, //! \todo
	{0x03,"JAL"}, //! \todo
	{0x04,"BEQ"}, //! \todo
	{0x05,"BNE"}, //! \todo
	{0x06,"BLEZ"}, //! \todo
	{0x07,"BGTZ"}, //! \todo
	{0x08,"ADDI"}, //! \todo
	{0x09,"ADDIU"}, //! \todo
	{0x0a,"SLTI"}, //! \todo
	{0x0b,"SLTIU"}, //! \todo
	{0x0c,"ANDI"},//! \todo
	{0x0d,"ORI"}, //! \todo
	{0x0e,"XORI"}, //! \todo
	{0x0f,"LUI"}, //! \todo
	{0x10,"ni"},
	{0x11,"ni"},
	{0x12,"ni"},
	{0x13,"ni"},
	{0x14,"nv"},
	{0x15,"nv"},
	{0x16,"nv"},
	{0x17,"nv"},
	{0x18,"nv"},
	{0x19,"nv"},
	{0x1a,"nv"},
	{0x1b,"nv"},
	{0x1c,"nv"},
	{0x1d,"nv"},
	{0x1e,"nv"},
	{0x1f,"nv"},
	{0x20,"LB"}, //! \todo
	{0x21,"LH"}, //! \todo
	{0x22,"LWL"}, //! \todo
	{0x23,"LW"}, //! \todo
	{0x24,"LBU"}, //! \todo
	{0x25,"LHU"}, //! \todo
	{0x26,"LWR"}, //! \todo
	{0x27,"nv"},
	{0x28,"SB"}, //! \todo
	{0x29,"SH"}, //! \todo
	{0x2a,"SWL"},
	{0x2b,"SW"}, //! \todo 26
	{0x2c,"nv"},
	{0x2d,"nv"},
	{0x2e,"SWR"}, //!
	{0x2f,"nv"},
	{0x30,"ni"},
	{0x31,"ni"},
	{0x32,"ni"},
	{0x33,"ni"},
	{0x34,"nv"},
	{0x35,"nv"},
	{0x36,"nv"},
	{0x37,"nv"},
	{0x38,"ni"},
	{0x39,"ni"},
	{0x3a,"ni"},
	{0x3b,"ni"},
	{0x3c,"nv"},
	{0x3d,"nv"},
	{0x3e,"nv"},
	{0x3f,"nv"}
};

uint32_t construct_R_bitstream(std::string rfunc, uint32_t src1, uint32_t src2, uint32_t dest, uint32_t shift_amt){
	uint32_t bitstream=0x00000000, opcode=0, fn_code = 0;
	// Find the fn code of the string
	fn_code = r_to_op.at(rfunc);
	// Find the rest of the bitstream
	bitstream = bitstream | (src1 << 21) | (src2 << 16) | (dest << 11) | (shift_amt<<6) | fn_code;
	return bitstream;
}

using namespace std;

struct result_set{
	int passed;
	string msg;

	result_set();
	result_set(int passed);
	result_set(int passed, string msg);

};

result_set::result_set(int passed_in, string msg_in){
	msg = msg_in;
	passed = passed_in;
}

result_set::result_set(){
	msg = "";
	passed = 0;
}

result_set::result_set(int passed_in){
	msg = "";
	passed = passed_in;
}

result_set test_add(mips_mem_h, mips_cpu_h);
result_set test_addi(mips_mem_h, mips_cpu_h);
result_set test_addiu(mips_mem_h, mips_cpu_h);

/*!
 * Defined a function pointer which takes in an int and returns a result set.
 * @param
 * @return
 */
typedef result_set (*tFunc)(mips_mem_h, mips_cpu_h);

map<int, string> nameTestMap = {
		{0, "<internal>"}
		/*{1,"ADD"},
		{2,"ADDI"},
		{3,"ADDIU"},
		{4,"ADDU"}
		*/
};
/*
map<int, function<result_set(mips_mem_h, mips_cpu_h)> > testMap = {
		{0, test_step},
		{1,test_add}
		{2,&test_addi},
		{3,&test_addiu}
};*/
map<string, uint8_t> str_to_op;

void set_debug_level(int argc, char* argv[], mips_cpu_h cpu);

int main(int argc, char* argv[])
{
	/*! Print out arguments */
    int i;
    for(i=0; i<argc; ++i)
    {
    	printf("Argument %d : %s\n", i, argv[i]);
    }

    /*! Reverse map of ops for easy creation of ops. */
    for (auto const &i: op_to_str) {
        	str_to_op[i.second] = i.first;
        }

	/*! Create memory */
	mips_mem_h mem = mips_mem_create_ram(4096);

	/*! Create cpu */
	mips_cpu_h cpu = mips_cpu_create(mem);

	/*! Set debug level */
	set_debug_level(argc,argv,cpu);

	cout << test_add(mem,cpu).msg << endl;

	//uint32_t xv=0x001100FF;
    //mips_error err=mips_mem_write(mem, 12, 4, (uint8_t*)&xv);
	// memory_handler, byte address, number of bytes, data (in bytes)
	/*
	string func = "SLL";
	uint32_t src1 = 15, src2 = 16, dest = 1, shift_amt = 2;

	uint32_t w = construct_R_bitstream(func,src1,src2,dest,shift_amt);
	std::cout << std::hex << w << " converted to ";
	w = __builtin_bswap32(w);
	cout << hex << w << endl;
	mips_error err;
	err = mips_mem_write(mem, 0, 4, (uint8_t*)&w);
	mips_cpu_step(cpu);

	func = "SLLV";
	src1 = 15; src2 = 16; dest = 1; shift_amt = 2;
	w = construct_R_bitstream(func,src1,src2,dest, shift_amt);
	std::cout << std::hex << w << " converted to ";
	w = __builtin_bswap32(w);
	cout << hex << w << endl;
	err = mips_mem_write(mem, 4, 4, (uint8_t*)&w);

	mips_cpu_step(cpu);
	*/
//	/*! Prepare for tests */
//	mips_test_begin_suite();
//
//	string testName;
//	int testId = 0;
//	result_set results;
//	int NUM_TESTS = 1;
//	/*! \todo Loop which runs through all tests in an array */
//	for (int i = 0;i<NUM_TESTS;i++)
//	{
//		testName = nameTestMap[i];
//
//		/*! Begin running test "string" */
//		testId=mips_test_begin_test(testName.c_str());
//		cout << "Test id is: " << testId << endl;
//
//		/*! Run tests and write results to result_set object */
//		results = testMap[i](mem,cpu);
//		fprintf(stdout,"Test %d done", testId);
//		/*! mips_test_end_test will get results from result_set */
//		mips_test_end_test(testId, results.passed, results.msg.c_str());
//
//	}
//	mips_test_end_suite();

	return 0;
}

void set_debug_level(int argc, char* argv[],mips_cpu_h cpu){
	unsigned level = 0;
	FILE * dest;
	const char* filename;
	for (int i=0;i<argc;i++){
		if (!strcmp(argv[i],"-d")){
			level = atoi(argv[i+1]);
		}
		if (!strcmp(argv[i],"-f")){
			filename = argv[i+1];
			if (!strcmp(filename,"stdout")){
				dest = stdout;
			}
			else if (!strcmp(filename,"stderr")){
				dest = stderr;
			}
			else{
				dest = fopen(filename, "w");
			}
		}
	}
	mips_cpu_set_debug_level(cpu,level,dest);
}

result_set test_sll(mips_mem_h mem, mips_cpu_h cpu){
	result_set results(0);
	// Write a sequence of instructions to memory, a list of uint32_t's, testing all funcs
	return results;
}

result_set mips_add(mips_mem_h mem, mips_cpu_h cpu, uint32_t loc, uint32_t a, uint32_t reg1, uint32_t b, uint32_t reg2, uint32_t dest, int p){
	result_set results;
	mips_error err = mips_Success;;
	string func="ADD";
	int shift_amt;
	uint32_t w = construct_R_bitstream(func,reg1,reg2,dest,shift_amt);
	w = __builtin_bswap32(w);
	// Write add instruction to memory at specified location
	err = mips_mem_write(mem, loc, 4, (uint8_t*)&w);
	err = mips_cpu_set_register(cpu,reg1,a);
	err = mips_cpu_set_register(cpu,reg2,b);
	err = mips_cpu_step(cpu);
	if (reg1==0){a = 0;}if(reg2==0){b=0;}
	uint32_t model_ans=a+b;
	uint32_t ans;
	err = mips_cpu_get_register(cpu,dest,&ans);
	if (dest==0){model_ans=0;}
	if(err!=mips_Success){return err;}
	if(p){fprintf(stdout, "%d + %d = %d, expected = %d\n", a, b, ans, model_ans);}
	if(model_ans!=ans){
	    fprintf(stderr, "Sum is wrong.\n");
	    return mips_InternalError;
	}
	if (err!=mips_Success){

	}
	return results;
}

result_set test_add(mips_mem_h mem, mips_cpu_h cpu){
	result_set results(0, "");
	mips_error err;
	uint32_t loc,reg1,reg2,dest;
	int32_t a,b;
	dest = 31;
	loc=0;
	err = mips_cpu_set_pc(cpu, loc);
	a = 1;b=-4;
		// Checking all the registers can add, and 0 reg is always 0.
	for (reg1 = 0; reg1 < 31; ++reg1) {
		for (reg2 = 0; reg2 < 31; ++reg2) {
			if (reg1==reg2){continue;}
			err = mips_add(mem,cpu,loc,a,reg1,b,reg2,dest,0);
			if (err==mips_Success){
				results.passed = 1;
				loc+=4;
			}
			else{
				results.passed =0;
				results.msg = err;
				return results;
			}
		}
	}
	cout << "first tests done" << endl;
	// Checking 0 reg is always 0
	dest = 0;
	err = mips_add(mem,cpu,loc,a,reg1,b,reg2,dest,1);
	loc +=4;

	// Test difficult adds on two set register
	reg1 = 2;
	reg2 = 3;
	dest = 1;
	a = 0x7FFFFFFF;b=0x7FFFFFFF;
	err = mips_add(mem,cpu,loc,a,reg1,b,reg2,dest,1);
	if (err==mips_Success){
		results.passed = 1;
		loc+=4;
	}
	else{
		results.passed =0;
		results.msg = err;
		return results;
	}
	a = 0x80000000;b=0x800000FF;
	err = mips_add(mem,cpu,loc,a,reg1,b,reg2,dest,1);
	if (err==mips_Success){
		results.passed = 1;
		loc+=4;
	}
	else{
		results.passed =0;
		results.msg = err;
		return results;
	}
	cout << "second tests done" << endl;

	return results;
}

result_set test_addi(mips_mem_h, mips_cpu_h){
	result_set results(0, "Fail");
	return results;
}

result_set test_addiu(mips_mem_h, mips_cpu_h){
	result_set results(0, "Fail");
	return results;
}





/*!
 * Writes a function into memory and sets registers to specified values.
 * @param mem - Memory handler (needed because don't know internals of CPU to access memory)
 * @param cpu - CPU handler
 * @param loc - Need to ensure PC==loc to ensure the instruction is executed.
 * @param func - String of R func to write to memory
 * @param reg1 - index of reg1
 * @param reg2 - index of reg2
 * @param shift_amt - shift amount (if shifting)
 * @param dest - index of destination register
 * @return
 */
mips_error mips_R_func(
		mips_mem_h mem,
		mips_cpu_h cpu,
		uint32_t loc,
		string func,
		uint32_t reg1,
		uint32_t reg2,
		uint32_t shift_amt,
		uint32_t dest){
	mips_error err = mips_Success;
	uint32_t w = construct_R_bitstream(func,reg1,reg2,dest,shift_amt);
	w = __builtin_bswap32(w);
	// Write instruction to memory at specified location
	err = mips_mem_write(mem, loc, 4, (uint8_t*)&w);
	if (err!=mips_Success){return err;}
	return err;
}

//xa[0]={0x00, 0x11, 0x00, 0xFF};

struct model_state{
	uint32_t pc;
	uint32_t regs[32];
	uint32_t hi;
	int32_t lo;
};

result_set compare_model(mips_cpu_h cpu, model_state model){
	result_set results(1,"");
	stringstream ss;
	mips_error err;
	uint32_t tmp, pc;
	for (unsigned i=0;i<32;i++){
		err = mips_cpu_get_register(cpu, i, &tmp);
		if (model.regs[i]!=tmp) {
			ss << "Register" << i << "is value" << tmp << ", should be value "
			<< model.regs[i] << endl;
			results.passed=0;
		}
	}
	err = mips_cpu_get_pc(cpu, &pc);
	if (pc!=model.pc){
		ss << "PC is " << pc << ". Should be " << model.pc << "." << endl;
		results.passed=0;
	}
	results.msg = ss.str();
	return results;
}

result_set mips_test_R_func(
		mips_mem_h mem,
		mips_cpu_h cpu,
		string func
		){
	mips_error err = mips_Success;
	uint32_t loc = 0, reg1 = 0, reg2 = 0, shift_amt = 0, dest = 31, a = 0, b = 0;
	model_state model;
	result_set results;
		// Reset CPU
		err = mips_cpu_reset(cpu);
		// Write function to specified location (location 0)
		err = mips_R_func(mem,cpu,loc,func,reg1,reg2,shift_amt,dest);
		// Set PC to that location
		err = mips_cpu_set_pc(cpu, loc);
		// Set registers
		err = mips_cpu_set_register(cpu, reg1, a);
		err = mips_cpu_set_register(cpu, reg2, b);
		// Step CPU (ensure PC is at specified location)
		err = mips_cpu_step(cpu);
		// Get model answer for values above.
		model = get_Rmodel(func);
		// Check answer in dest with model answer
		results = compare_model(cpu,model);
		if (results.passed==0){
			return results;
		}
	return results;
}

model_state get_Rmodel(mips_cpu_h cpu, string func,uint32_t reg1,uint32_t reg2,uint32_t dest, uint32_t shift_amt, uint32_t hi, uint32_t lo){
	model_state model;
	uint32_t tmp,pc,s,t;
	uint32_t * d;
	mips_error err;
	// Copy the state of the pc just before executing the instruction
	for (unsigned i=0;i<32;i++){
		err = mips_cpu_get_register(cpu, i, &tmp);
		model.regs[i]=tmp;
	}
	model.hi = hi;
	model.lo = lo;
	err = mips_cpu_get_pc(cpu, &pc);
	s = (model.regs[reg1]);
	t = (model.regs[reg2]);
	d = &(model.regs[dest]);
	// Obtain correct answer given parameters above
	if (func == "ADD"||func=="ADDU"){
		*d = s+t;
	}
	else if (func=="SUB"||func=="SUBU"){
		*d = s-t;
	}
	else if (func=="JR"||func=="JRAL"){
		model.pc = s;
		if (func=="JRAL"){model.regs[31]=pc+8;}
		return model;
	}
	else if (func=="MULT"||"MULTU"){
		// Top part of multiplication
		uint64_t tmp;
		tmp = (uint64_t)s*(uint64_t)t;
		model.hi = (uint32_t)((tmp>>32)&0xFFFFFFFF);
		model.lo = (int32_t)(tmp&0xFFFFFFFF);
	}
	else if (func=="DIV"){
		model.lo = (int32_t)s/(int32_t)t;
		model.hi = s/t;
	}
	else if (func=="DIVU"){
		model.lo = s/t;
		model.hi = s/t;
	}
	else if (func==)
	model.pc = pc+4;
	return model;
}

if (reg1==0){a = 0;}if(reg2==0){b=0;}
	uint32_t model_ans=a+b;
	uint32_t ans;
	err = mips_cpu_get_register(cpu,dest,&ans);
	if (dest==0){model_ans=0;}
	if(err!=mips_Success){return err;}
	if(p){fprintf(stdout, "%d + %d = %d, expected = %d\n", a, b, ans, model_ans);}
	if(model_ans!=ans){
	    fprintf(stderr, "Sum is wrong.\n");
	    return results;
	}
	if (err!=mips_Success){
		results.msg = err;
	}
