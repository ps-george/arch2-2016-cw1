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
#include "test_mips.h"
#include <iomanip>
#include <fstream>
using namespace std;

/*
* MAP DEFINITIONS
*/
const std::map<std::string, uint32_t> r_to_op{
	{"SLL",0x0},
	{"SRL",0x02},
	{"SRA",0x03},
	{"SLLV",0x04},
	{"SRLV",0x06},
	{"SRAV",0x07},
	{"JR",0x08},
	{"JALR",0x09},
	{"MFHI",0x10},
	{"MTHI",0x11},
	{"MFLO",0x12},
	{"MTLO",0x13},
	{"MULT",0x18},
	{"MULTU",0x19},
	{"DIV",0x1a},
	{"DIVU",0x1b},
	{"ADD",0x20},
	{"ADDU",0x21},
	{"SUB",0x22},
	{"SUBU",0x23},
	{"AND",0x24},
	{"OR",0x25},
	{"XOR",0x26},
	{"NOR",0x27},
	{"SLT",0x2a},
	{"SLTU",0x2b}
};

const std::map<std::string, uint32_t> rt_to_op{
	{"BLTZ",0x0},
	{"BGEZ",0x1},
	{"BLTZAL", 0x10},
	{"BGEZAL", 0x11}
};

const std::map<uint32_t, std::string> op_to_str{
	{0x02,"J"},
	{0x03,"JAL"},
	{0x04,"BEQ"},
	{0x05,"BNE"},
	{0x06,"BLEZ"},
	{0x07,"BGTZ"},
	{0x08,"ADDI"},
	{0x09,"ADDIU"},
	{0x0a,"SLTI"},
	{0x0b,"SLTIU"},
	{0x0c,"ANDI"},
	{0x0d,"ORI"},
	{0x0e,"XORI"},
	{0x0f,"LUI"},
	{0x20,"LB"},
	{0x21,"LH"},
	{0x22,"LWL"},
	{0x23,"LW"},
	{0x24,"LBU"},
	{0x25,"LHU"},
	{0x26,"LWR"},
	{0x28,"SB"},
	{0x29,"SH"},
	{0x2a,"SWL"},
	{0x2b,"SW"},
	{0x2e,"SWR"}
};
/*
 * END OF MAP DEFINITIONS
 */
/*
 * STRUCT DEFINITIONS
 */
struct model_state{
	uint32_t pc;
	uint32_t regs[32];
	model_state(mips_cpu_h cpu, uint32_t val, uint32_t dest, uint32_t Npc);
	model_state();
};

// Model based on actual state of the CPU, except with desired values inside.
model_state::model_state(mips_cpu_h cpu, uint32_t dest, uint32_t val, uint32_t Npc){
	uint32_t tmp_val;
	mips_error err;
	for (unsigned i=0;i<32;i++){
		err = mips_cpu_get_register(cpu, i, &tmp_val);
		if (err!=mips_Success){cout << "Error reading register " <<i<<endl;}
		regs[i] = tmp_val;
	}
	regs[dest] = val;
	pc = Npc;
}

model_state::model_state(){
	pc = 0;
	for (unsigned i=0;i<32;i++){
		regs[i] = 0;
	}
}

struct result_set{
	int passed;
	string msg;
	uint32_t ans;
	uint32_t ans2;
	result_set();
	result_set(int passed);
	result_set(int passed, string msg_in);
};

result_set::result_set(int passed_in, string msg_in){
	msg = msg_in;
	passed = passed_in;
	ans = 0;
	ans2 = 0;
}
result_set::result_set(int passed_in){
	msg = "";
	passed = passed_in;
	ans = 0;
	ans2 = 0;
}
/*
* END OF STRUCT DEFINITIONS
*/

/*
* MAIN FUNCTION
*/
int main(int argc, char* argv[])
{
	/*! Print out arguments */
    int i;
    for(i=0; i<argc; ++i)
    {
    	printf("Argument %d : %s\n", i, argv[i]);
    }

    // Reverse map of opcodes for easy creation of opcodes.
    for (auto const &i: op_to_str) {
        	ij_to_op[i.second] = i.first;
        }

	// Create memory
    uint32_t mem_size = 0x2000;
	mips_mem_h mem = mips_mem_create_ram(mem_size);
	cout << "Memory size: " << mem_size << endl;

	// Create cpu
	mips_cpu_h cpu = mips_cpu_create(mem);

	// Get & Set debug level
	set_debug_level(argc,argv,cpu);
	// Prepare for tests
	mips_test_begin_suite();
	// Test that all 32 registers exist and contain uint32_t

	// Read test_spec into vector of strings
	vector<vector<string> > spec1;
	// Could pass debug level into this function and use it to help debug the test program.
	parse_test_spec("mips_test_spec.csv", spec1);

	// Execute test spec
	run_spec(spec1, mem,cpu);
	mips_test_end_suite();
	return 0;
}
/*
* END OF MAIN
*/

/*
* GENERAL HELPER FUNCTIONS
*/
// Function which takes in a string naming the test file, tests all the cases in the file
// PARSE TEST SPEC .CSV
int parse_test_spec(string filename, vector<vector<string> > &spec){
	string line;
	ifstream test_file(filename);
	// Check test file was opened.
	if (test_file.is_open()){
		cout << "Test file opened." << endl;
	}
	else{
		cout << "Could not open test file." << endl;
		return 1;
	}

	int i = 0;
	while (getline(test_file,line)){
		std::istringstream s(line);
		std::string field;
		vector<string> vec;
		spec.push_back(vec);
		while (getline(s, field,',')){
			spec[i].push_back(field);
		}
		i++;
	}
	test_file.close();
	return 0;
}
// END OF PARSE TEST SPEC .csv
// SET DEBUG LEVEL
int set_debug_level(int argc, char* argv[],mips_cpu_h cpu){
	unsigned level = 0;
	FILE * dest;
	const char* filename;
	if (argc<3){
		return 0;
	}
	if (!strcmp(argv[1],"-d")){
		level = atoi(argv[2]);
		dest = stdout;
		if (argc>4&&(!strcmp(argv[3],"-f"))){
			filename = argv[4];
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
	cout << "Set debug level to: " << level << ". Print output to: " << filename;
	mips_cpu_set_debug_level(cpu,level,dest);
	return level;
}
// END OF SET DEBUG LEVEL

/*!
 * Take in a list of params and function name and construct a bitstream for MIPS from it.
 * @param params - s,t,d,h for R-type; s,i for RT-type; j for J type; s,t,i for I type
 * @param func - string containing the function name
 * @return uint32_t bigendian bitstream to be written to memory.
 */
// CONSTRUCT BIT STREAM
uint32_t test_construct_bitstream(string func, const uint32_t type, const vector<uint32_t> &params){
	uint32_t bitstream=0x0;
	uint32_t s,t,d,h,i,j;
	//! 31:26, 25:21,
	switch(type){
	case instr_R_type: // R-type
		s = params[0]&0x1F;
		t = params[1]&0x1F;
		d = params[2]&0x1F;
		h = params[3]&0x1F;
		bitstream = 0x0 |  (s << 21) | (t << 16) | (d << 11) | (h<<6) | r_to_op.at(func);
		break;
	case instr_RT_type: // RT-type
		s = params[0];
		i = params[1];
		bitstream = 0x04000000 | (s << 21) | ((rt_to_op.at(func)&0xFFFF) << 16)| (i&0xFFFF);
		break;
	case instr_J_type: // J type
		j = params[0];
		bitstream = ij_to_op.at(func)<<26|(j&0x03FFFFFF);
		break;
	case instr_I_type: //I-type
		s = params[0];
		t = params[1];
		i = params[2];
		bitstream = ij_to_op.at(func)<<26|(s<<21)|(t<<16)|(i&0xFFFF);
		break;
	}
	bitstream = __builtin_bswap32(bitstream);
	return bitstream;
}
// END OF CONSTRUCT bitstream
// COMPARE MODEL
int compare_model(mips_cpu_h cpu, model_state model, result_set &results){
	mips_error err;
	uint32_t tmp, pc;
	for (unsigned i=0;i<32;i++){
		err = mips_cpu_get_register(cpu, i, &tmp);
		if (err!=mips_Success){
			results.passed=0;
			return 1;
		}
		if (model.regs[i]!=tmp) {
			cout << hex << "Reg " << i << " = 0x" << tmp << ". Should be 0x" << model.regs[i] << "." << dec <<endl;
			results.passed=0;
			return 1;
		}
	}
	err = mips_cpu_get_pc(cpu, &pc);
	if (mips_test_check_err(err, results)){return 1;}
	if (pc!=model.pc){
		cout << "PC = " << pc << ". Should be " << model.pc<< endl;
		results.passed=0;
		return 1;
	}
	return 0;
}
// END OF COMPARE MODEL
// MIPS TEST CHECK ERR
int mips_test_check_err(mips_error err, result_set &results){
	if (err!=mips_Success){
		results.passed = 0;
		//results.msg << "Mips error code: " << err;
		return 1;
	}
	return 0;
}
// END OF MIPS TEST CHECK ERR
/*
* END OF GENERAL HELPER FUNCTIONS
*/
/*
*	RUNNING THE TEST SPEC
*/
//! This will replace run_spec when it's child functions are ready
void run_spec(const vector<vector<string>> &spec, mips_mem_h mem, mips_cpu_h cpu){
	// Iterate through spec and execute tests accordingly
	// from now one func will be in row 2 and message will be in row 1
	uint32_t type;
	string func = spec[0][2];
	int testId;
	for (unsigned i=0;i<spec.size();i++){
		if ((spec[i][0]=="")||
			(spec[i][0]=="Rnorm") ||
			(spec[i][0]=="Rmdiv") ||
			(spec[i][0]=="Rmtmf") ||
			(spec[i][0]=="Rbranch") ||
			(spec[i][0]=="Rtbranch") ||
			(spec[i][0]=="Jbranch") ||
			(spec[i][0]=="Ibranch") ||
			(spec[i][0]=="Inorm") ||
			(spec[i][0]=="Imemread")
					){
			// Ignore header rows
			// cout << "Ignore " << spec[i][0] << " header row." << endl;
			continue;
		}
		if (spec[i][0]=="STOP"){
			return;
		}
		func = spec[i][2];
		// get the last byte of type
		type = 0x0F&s_to_ui(spec[i][0]);
		// Reset results - [i][1] contains the message
		result_set results(1,spec[i][1]);
		// Begin test
		testId = mips_test_begin_test(func.c_str());
		// Reset CPU
		mips_cpu_reset(cpu);
		// cout << "0x" << hex << type << dec << endl;
		switch(type){
		case test_Normal: // 0x1
			test_normal_functions(spec[i],results,mem,cpu);
			break;
		case test_Branch: //0x2
			test_branch_functions(spec[i],results,mem,cpu);
			break;
		case test_MemWrite: //0x3
			//test_memory_write_functions(spec[i],results,mem,cpu);
			break;
		case test_MemRead: //0x4
			test_memory_read_functions(spec[i],results,mem,cpu);
			break;
		case test_MTMF: //0x5
			test_mtmf_functions(spec[i],results,testId,mem,cpu);
			continue;
		case test_MultDiv: //0x6
			test_multdiv_functions(spec[i], results,testId, mem, cpu);
			// MULT/DIV will end its tests inside the function
			continue;
		default:
			results.passed = 0;
			cout << "Didn't recognise type " << type << "skipping test." << endl;
			continue;
		}
		if(!results.passed){
			cout << "Test " << testId << ", " << func <<". Testing " << results.msg << ". Result:" << results.passed << endl;
		}
		mips_test_end_test(testId, results.passed, results.msg.c_str());
	}
}

//! These are the simplest instructions to test, just need to check dest reg for correct value or error code
void test_normal_functions(const vector<string> &row, result_set &results, mips_mem_h mem, mips_cpu_h cpu){
	// Initialise
	uint32_t byte2 = s_to_ui(row[0])>>4;
	uint32_t s,t,h,i,t_val,s_val,ans,exp_err,dest;
	uint32_t instruction_bits;
	string func = row[2];
	mips_error err = mips_Success;
	vector<uint32_t> params;
	switch(byte2){
	case instr_R_type:
		// This would be R-type functions
		s = s_to_ui(row[3]);
		t = s_to_ui(row[4]);
		dest = s_to_ui(row[5]);
		h = s_to_ui(row[6]);
		s_val = s_to_ui(row[7]);
		t_val = s_to_ui(row[8]);
		ans = s_to_ui(row[9]);
		exp_err = s_to_ui(row[10]); // If error is set, check for error instead of the answer
		// Create r-type bitstream
		params = {s,t,dest,h};
		instruction_bits = test_construct_bitstream(func,instr_R_type, params);
		// Set t register
		// Set d_val to non-zero because it should be overwritten
		err = mips_cpu_set_register(cpu, dest, 11);
		err = mips_cpu_set_register(cpu, t, t_val);
		break;
	case instr_RT_type:
		cout << "RT-type instruction ended up in Normal handler, shouldn't be here." << endl;
		results.passed = 0;
		return;
	case instr_I_type:
		// This would be I-type functions
		s = s_to_ui(row[3]);
		dest = s_to_ui(row[4]); // t is the destination
		i = s_to_ui(row[5]);
		s_val = s_to_ui(row[6]);
		ans = s_to_ui(row[7]);
		exp_err = s_to_ui(row[8]);
		params = {s,dest,i};
		instruction_bits = test_construct_bitstream(func, instr_I_type, params);
		err = mips_cpu_set_register(cpu, dest, 11);
		break;
	case instr_J_type:
		// This would be J-type functions but again none in the normal testing category
		cout << "J-type instruction ended up in Normal handler, shouldn't be here." << endl;
		results.passed = 0;
		return;
	default:
		cout << "Strange instruction type code ended up in Normal handler, shouldn't be here." << endl;
		results.passed = 0;
		return;
	}

	// We always set s register because it is common to I and R-type
	err = mips_cpu_set_register(cpu, s, s_val);
	// Write bitstream to memory
	err = mips_mem_write(mem, 0, 4,(uint8_t*)&instruction_bits);
	// Create model
	model_state model(cpu,dest,ans,4);
	// Step CPU
	err = mips_cpu_step(cpu);
	if (exp_err||err){
		if (err != exp_err){
			results.passed = 0;
			cout << hex << "Incorrect error message " << err << " was expecting " << exp_err << dec <<endl;
		}
		return;
	}
	// Create model_state
	// nPC is always +4 for these "normal" instructions, and we will always write the instruction to 0 in memory.
	compare_model(cpu, model, results);
	return;
}

//! These are the most complicated instructions to test, involving multiple steps and checking.
void test_branch_functions(const vector<string> &row, result_set &results, mips_mem_h mem, mips_cpu_h cpu){
	// Initialise variables
	//! dest is the location to write the link location to, normally 31
	uint32_t s,t,dest,d,h,j,t_val,s_val,target,loc,link,exp_err3,branch;
	exp_err3 = 0;dest=0;link=0;loc=0;branch=1;
	int32_t i;
	mips_error err = mips_Success;
	uint32_t instruction_bits;
	uint32_t byte2 = s_to_ui(row[0])>>4;
	string func = row[2];
	vector<uint32_t> params;
	// Different variations of this code are repeated each time, can they be put into one function?
	// I guess I could refer to the vector directly and not name the variables, but then I lose clarity
	// I like the clarity of naming each of the variables otherwise I might get confused.
	switch(byte2){
	case instr_R_type:
		// This would be R-type functions
		//! \todo Bottom two bits of s can't be 1 test
		s = s_to_ui(row[3]); // s contains the value to jump to
		dest = s_to_ui(row[4]); // t_reg - want to test == 0 corner case
		d = s_to_ui(row[5]); // only used in error msg test
		h = s_to_ui(row[6]);
		target = s_to_ui(row[7]); // s_val
		link = s_to_ui(row[8]);
		exp_err3 = s_to_ui(row[9]);
		branch = 1; // For R type, we only jump forwards, so write to 0 location
		// Create r-type bitstream (d,h are usually 0, included to test corner case)
		params = {s,dest,d,h};
		instruction_bits = test_construct_bitstream(func,instr_R_type,params);
		// Set s to value of target
		err = mips_cpu_set_register(cpu, s, target);
		break;
	case instr_RT_type:
		// This would be RT-type functions, these compare s with 0.
		dest = 31;
		s = s_to_ui(row[3]);
		// t = s_to_i(row[4]);
		i = s_to_i(row[5]); // sign extend i
		s_val = s_to_ui(row[6]);
		link = s_to_ui(row[7]);
		loc = s_to_ui(row[8]);
		branch = s_to_ui(row[9]);
		exp_err3 = s_to_ui(row[10]);
		target = (int32_t)(loc+4) + (i<<2);
		params = {s,s_to_ui(row[5])};
		instruction_bits = test_construct_bitstream(func, instr_RT_type, params);
		err = mips_cpu_set_register(cpu,s,s_val);
		// err = mips_cpu_set_register(cpu,t,t_val);
		break;
	case instr_I_type:
		// branch depends on s and t and s_val and t_val
		s = s_to_ui(row[3]);
		t = s_to_ui(row[4]);
		i = s_to_i(row[5]); // sign extend i
		s_val = s_to_ui(row[6]);
		t_val = s_to_ui(row[7]);
		loc = s_to_ui(row[8]);
		branch = s_to_ui(row[9]);
		exp_err3 = s_to_ui(row[10]);
		target = (int32_t)(loc+4) + (i<<2);
		//cout << "Target: 0x" << hex <<target <<dec<< endl;
		// create I type bitstream
		params = {s,t,s_to_ui(row[5])};
		instruction_bits = test_construct_bitstream(func, instr_I_type, params);
		//cout << "Instruction bits: 0x" << hex << instruction_bits << dec << endl;
		// Set s_val and t_val
		err = mips_cpu_set_register(cpu,s,s_val);
		err = mips_cpu_set_register(cpu,t,t_val);
		break;
	case instr_J_type:
		dest = 31;
		j = s_to_ui(row[3]);
		target = j << 2;
		//cout << "Jumping to address: 0x" << hex << target << dec << endl;
		link = s_to_ui(row[4]);
		exp_err3 = s_to_ui(row[5]);
		// create j-type bitstream
		params = {j};
		instruction_bits = test_construct_bitstream(func, instr_J_type, params);
		break;
	default:
		cout << "Strange instruction type code ended up in branch handler, shouldn't be here." << endl;
		results.passed = 0;
		return;
	}
	if (link){
		link = loc + 8;
		err = mips_cpu_set_register(cpu,dest,1);
	}
	// Write jump/branch instruction to loc
	err = mips_mem_write(mem, loc, 4, (uint8_t*)&instruction_bits);

	err = mips_cpu_set_pc(cpu,loc);

	// If expecting error in the first step - no longer expecting errors in first step, errors normally occur when instruction is read from memory
	/*
	if (exp_err1){
		err = mips_cpu_step(cpu);
		if (err!=exp_err1){
			results.passed = 0;
			cout << hex << "Incorrect error message " << err << " was expecting " << exp_err1 << dec << endl;
		}
		return;
	}
	*/
	//! Create delay slot instruction.
	uint32_t d_reg,d_ans;
	d_ans = 5;
	d_reg = 6;
	// Set registers
	err = mips_cpu_set_register(cpu,4,3);
	err = mips_cpu_set_register(cpu,5,2);
	params = {4,5,d_reg,0};
	instruction_bits = test_construct_bitstream("ADDU",instr_R_type,params);
	// After the first step, nothing will have changed except the PC must be +4 and reg[dest]==link address]
	model_state model(cpu,dest,link,loc+4);
	// Write add instruction to delay slot location
	err = mips_mem_write(mem, loc+4, 4, (uint8_t*)&instruction_bits);
	err = mips_cpu_step(cpu);
	//cout << "Error code: 0x" << hex << err <<dec << endl;
	// If the cpu state doesn't match the model, return, no further testing.
	if (compare_model(cpu,model,results)){
		cout << "Failed at first step of branch instruction." << endl;
		return;
	}
	// After the second step, the delay slot instruction must execute (always do a simple add for this)
	model_state model2(cpu,d_reg,d_ans,loc+8);
	err = mips_cpu_step(cpu);
	// if not meant to branch, check that delay slot instruction executed, and that pc = loc + 8
	if (!branch){
		if (compare_model(cpu,model2,results)){
			cout << "Failed at second step of non-branching instruction." << endl;
		}
		// step again, check pc only incremented by 4
		mips_cpu_step(cpu);
		uint32_t tmp_pc;
		err = mips_cpu_get_pc(cpu,&tmp_pc);
		if (tmp_pc!=loc+12){
			cout << "Failed at third step of non-branching instruction." << endl;
			results.passed=0;
		}
		return;
	}
	// PC must be = target. Also reg[dest] must equal PC+8, but we did that earlier
	model2.pc = target;
	if (compare_model(cpu,model2,results)){
		cout << "Failed at second step of branch instruction." << endl;
		return;
	}

	uint32_t j_ans = 9;
	uint32_t j_reg = 18;
	// Set registers
	err = mips_cpu_set_register(cpu,16,4);
	err = mips_cpu_set_register(cpu,17,5);
	params = {16,17,j_reg,0};
	instruction_bits = test_construct_bitstream("ADDU",instr_R_type,params);
	// Write add instruction to jump location
	// After the third step, the PC must be target + 4, another simple add must have the correct answer
	// and reg[dest] must be the same as before
	err = mips_mem_write(mem, target, 4, (uint8_t*)&instruction_bits);
	model_state model3(cpu,j_reg,j_ans,target+4);
	err = mips_cpu_step(cpu);
	// If expecting error after the 3rd step
	if (exp_err3){
		err = mips_cpu_step(cpu);
		if (err!=exp_err3){
			results.passed = 0;
			cout << hex << "Incorrect error message " << err << " was expecting " << exp_err3 << dec <<endl;
		}
		return;
	}
	if (compare_model(cpu,model3,results)){
		cout << "Failed at third step of branch instruction." << endl;
	}
	return;
}

//! These are quite simple to test, need to write a specified value to a memory location using the cpu
//! then check the memory for correct written value. These are all I-type instructions.
void test_memory_write_functions(const vector<string> &row, result_set &results, mips_mem_h mem, mips_cpu_h cpu){
	cout << "Got to memory write functions, not ready yet, fail test." << endl;
	results.passed = 0;
	return;

	// Initialise variables


	// Create bitstream for specified write to memory instruction

	// Check memory location for value written to memory
	return;
}

//! These are quite simple to test. Write specified value to memory location using mips_mem_write
//! Then check the memory for correct value using cpu function. Again, all I-type functions
void test_memory_read_functions(const vector<string> &row, result_set &results, mips_mem_h mem, mips_cpu_h cpu){
	mips_error err = mips_Success;
	// Do we have to check reading to different registers? Maybe, read to dest reg = 0.
	// s is address, t is destination, i is offset (s + offset is effective address)
	// Initialise variables
	uint32_t s, t, i, b, ans, exp_err,instruction_bits, memloc, big_b, s_val;
	// Write 4 bytes to specific location in memory
	string func = row[2];
	// Assign variables
	s = s_to_ui(row[3]);
	t = s_to_ui(row[4]); // destination
	i = s_to_ui(row[5]);
	s_val = s_to_ui(row[6]);
	b = s_to_ui(row[7]);
	memloc = s_to_ui(row[8]);
	ans = s_to_ui(row[9]);
	exp_err = s_to_ui(row[10]);
	vector<uint32_t> params = {s,t,i};
	// Set values of s register
	mips_cpu_set_register(cpu,s,s_val);

	// Write b to specific memory location memloc
	big_b = __builtin_bswap32(b);
	mips_mem_write(mem, memloc,4, (uint8_t*)&big_b);

	// Create read mem instruction at 0
	instruction_bits = test_construct_bitstream(func, instr_I_type,params);

	// Create model
	model_state model(cpu,t,ans,4);

	// Write instruction to memory
	mips_mem_write(mem, 0, 4, (uint8_t*)&instruction_bits);

	// Step pc
	err = mips_cpu_step(cpu);
	// If expecting error, check for error
	if (exp_err||err){
		if (err!=exp_err){
			results.passed=0;
			cout << hex << "Incorrect error message " << err << " was expecting " << exp_err << dec <<endl;
		}
		return;
	}
	//

	// Compare model to cpu state
	if(compare_model(cpu,model,results)){
		uint32_t tmp;
		mips_cpu_get_register(cpu, t, &tmp);
	}
	return;
}

//! These are fairly complicated to test, since they require multiple steps through the CPU.
//! Require MFHI MFLO to be working in order to test.
void test_multdiv_functions(const vector<string> &row, result_set &results,int testId, mips_mem_h mem, mips_cpu_h cpu){
	//cout << "Got to multdiv functions, not ready yet, fail test." << endl;
	//results.passed = 0;
	//mips_test_end_test(testId,results.passed,results.msg.c_str());
	//return;
	// Initialise variables
	uint32_t s,t,d,h,s_val, t_val,hi,lo, err_exp; //! s_val * t_val or s_val / t_val
	mips_error err = mips_Success;
	uint32_t instruction_bits;
	string func = row[2];
	stringstream ss;

	// Get variables from vector
	s = s_to_ui(row[3]);
	t = s_to_ui(row[4]);
	d = s_to_ui(row[5]);
	h = s_to_ui(row[6]);
	s_val = s_to_ui(row[7]);
	t_val = s_to_ui(row[8]);
	hi = s_to_ui(row[9]);
	lo = s_to_ui(row[10]);
	err_exp = s_to_ui(row[11]);
	vector<uint32_t> params = {s,t,d,h};

	// Set s and t values
	err = mips_cpu_set_register(cpu,s,s_val);
	err = mips_cpu_set_register(cpu,t,t_val);

	// Create bitstream
	instruction_bits = test_construct_bitstream(func,instr_R_type,params);

	// Write function to memory
	err = mips_mem_write(mem,0,4,(uint8_t*)&instruction_bits);
	model_state model(cpu,0,0,4);
	err = mips_cpu_step(cpu);

	// If expecting an error, check here.
	if (err_exp){
		if (err!=err_exp){
			results.passed = 0;
		}
		mips_test_end_test(testId, results.passed, results.msg.c_str());
		return;
	}
	// Nothing should have changed except PC
	if (compare_model(cpu,model,results)){
		cout << func << " failed at first step." << endl;
		mips_test_end_test(testId, results.passed, results.msg.c_str());
	}
	// Write MFHI function to memory.
	uint32_t d1,d2;
	d1 = 16;
	d2 = 17;
	params = {0,0,d1,0};
	instruction_bits = test_construct_bitstream("MFHI", instr_R_type, params);
	mips_mem_write(mem,4,4,(uint8_t*)&instruction_bits);
	// Save CPU state before step
	model_state model1(cpu,d1,hi,8);
	// Step CPU
	err = mips_cpu_step(cpu);
	// Compare with model
	int step2 = compare_model(cpu,model1, results);
	if (step2){
		//! MFHI failed OR MULT/DIV failed. Fail both tests.
		mips_test_end_test(testId, results.passed, results.msg.c_str());
		testId = mips_test_begin_test("MFHI");
		ss << "Testing " << func << " and MFHI value was wrong. Failing MFHI as well under this test.";
		mips_test_end_test(testId, 0,ss.str().c_str());
		return;
	}

	// Write MFLO function to memory
	params[2] = d2;
	instruction_bits = test_construct_bitstream("MFLO", instr_R_type, params);
	mips_mem_write(mem,8,4,(uint8_t*)&instruction_bits);
	// Save CPU state with desired values
	model_state model2(cpu,d2,lo,12);
	// Step CPU
	err = mips_cpu_step(cpu);
	// Compare with model

	if (compare_model(cpu,model2,results)){
		//! MFLO failed or MULT/DIV failed. Fail both tests.
		mips_test_end_test(testId, results.passed, results.msg.c_str());
		testId = mips_test_begin_test("MFLO");
		ss << "Testing " << func << " and MFLO value was wrong. Failing MFLO as well under this test.";
		mips_test_end_test(testId,0,ss.str().c_str());
		if (!step2){ // MFHI worked, even if MFLO/MULT/DIV failed.
			testId = mips_test_begin_test("MFHI");
			ss << "Testing " << func << " and MFHI value was correct. Passing MFHI under this test.";
			mips_test_end_test(testId, results.passed, ss.str().c_str());
		}
		return;
	}


	// OR, everything passed.
	// End mult/div test
	mips_test_end_test(testId, results.passed, results.msg.c_str());
	testId = mips_test_begin_test("MFLO");
	ss << "Testing " << func << " and MFLO value was correct. Passing MFLO under this test.";
	mips_test_end_test(testId, results.passed, ss.str().c_str());
	testId = mips_test_begin_test("MFHI");
	ss << "Testing " << func << " and MFHI value was correct. Passing MFHI under this test.";
	mips_test_end_test(testId, results.passed, ss.str().c_str());
	return;
}

//! These functions are difficult to test
void test_mtmf_functions(const vector<string> &row, result_set &results, int testId, mips_mem_h mem, mips_cpu_h cpu){
	// Initialise variables
	uint32_t s,t,d,h,ans, err_exp, d2_reg; //! MT takes from s_val, MF write to d
	mips_error err = mips_Success;
	uint32_t instruction_bits;
	string func = row[2];
	s = s_to_ui(row[3]);
	t = s_to_ui(row[4]);
	d = s_to_ui(row[5]);
	h = s_to_ui(row[6]);
	ans = s_to_ui(row[7]);
	err_exp = s_to_ui(row[8]);

	// Set s reg to s_val
	err = mips_cpu_set_register(cpu,s,ans);

	vector<uint32_t> params = {s,t,d,h};
	//construct bitstream
	instruction_bits = test_construct_bitstream(func, instr_R_type, params);
	// Write bitstream to memory
	err = mips_mem_write(mem,0,4,(uint8_t*)&instruction_bits);
	// Step the cpu once. PC should be 4, no registers should have changed.
	model_state model(cpu,0,0,4);
	//! Move the value from s_reg to hi/lo register.
	err = mips_cpu_step(cpu);
	if (err_exp){
		if (err!=err_exp){
			results.passed = 0;
		}
		// end test
		mips_test_end_test(testId, results.passed, results.msg.c_str());
		return;
	}
	if (compare_model(cpu,model,results)){
		cout << "Failed at first step." << endl;
		// End test
		mips_test_end_test(testId, results.passed, results.msg.c_str());
		return;
	}
	d2_reg = 16;
	// Construct bitsream for MFLO/MFHI depending
	vector<uint32_t> params2 = {0,0,d2_reg,0};
	string func2;
	if (func=="MTHI"){
		func2 = "MFHI";
		instruction_bits = test_construct_bitstream(func2, instr_R_type,params2);
	}
	else if (func=="MTLO"){
		func2 = "MFLO";
		instruction_bits = test_construct_bitstream(func2, instr_R_type,params2);
	}
	// Write instruction to memory
	err = mips_mem_write(mem,4,4,(uint8_t*)&instruction_bits);
	// Have stepped twice, pc should be 8. d2_reg should be equal to the correct answer. All other registers unchanged.
	model_state model1(cpu,d2_reg,ans,8);
	//! Move value from hi/lo register to d2_reg
	err = mips_cpu_step(cpu);
	if (compare_model(cpu,model1,results)){
		stringstream ss;
		cout << "Failed at second step." << endl;
		results.passed = 0;

		cout << "Test " << testId << ", " << func <<". Testing " << results.msg << ". Result:" << results.passed << endl;

		mips_test_end_test(testId, results.passed, results.msg.c_str());
		//! Failure could also have been caused by MFHI/MFLO
		testId = mips_test_begin_test(func2.c_str());
		ss << func << " and answer was incorrect. Failing " << func2;
		results.msg = ss.str();
		results.passed = 0;
		cout << "Test " << testId << ", " << func2 <<". Testing " << results.msg << ". Result:" << results.passed << endl;
		mips_test_end_test(testId,results.passed,results.msg.c_str());
		return;
	}
	mips_test_end_test(testId,results.passed,results.msg.c_str());
	return;
}
