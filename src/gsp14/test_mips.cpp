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
	mips_mem_h mem = mips_mem_create_ram(4096);

	// Create cpu
	mips_cpu_h cpu = mips_cpu_create(mem);

	// Get & Set debug level
	int debugLevel = set_debug_level(argc,argv,cpu);
	// Prepare for tests
	mips_test_begin_suite();
	// Test that all 32 registers exist and contain uint32_t

	// Read test_spec into vector of strings
	vector<vector<string> > spec1;
	// Could pass debug level into this function and use it to help debug the test program.
	parse_test_spec("mips_test_spec.csv", spec1);

	// Execute test spec
	run_spec2(spec1, mem,cpu);
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
	//cout << func << endl;
	switch(type){
	case instr_R_type: // R-type
		s = params[0];
		t = params[1];
		d = params[2];
		h = params[3];
		bitstream = 0x0 |  (s << 21) | (t << 16) | (d << 11) | (h<<6) | r_to_op.at(func);
		break;
	case instr_RT_type: // RT-type
		s = params[0];
		i = params[1];
		bitstream = 0x04000000 | (s << 21) | (rt_to_op.at(func) << 16)| i;
		break;
	case instr_J_type: // J type
		j = params[0];
		bitstream = ij_to_op.at(func)<<26|j;
		break;
	case instr_I_type: //I-type
		s = params[0];
		t = params[1];
		i = params[2];
		bitstream = ij_to_op.at(func)<<26|(s<<21)|(t<<16)|i;
		break;
	}
	cout << "0x" << hex << bitstream;
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
			//results.msg << "Error reading from register " << i << endl;
			results.passed=0;
			return 1;
		}
		if (model.regs[i]!=tmp) {
			//results.msg << "Register" << i << "is value" << tmp << ", should be value "
			//<< model.regs[i] << endl;
			results.passed=0;
		}
	}
	err = mips_cpu_get_pc(cpu, &pc);
	if (mips_test_check_err(err, results)){return 1;}
	if (pc!=model.pc){
		cout << "PC = " << pc << ". Should be " << model.pc<< endl;
		results.passed=0;
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
void run_spec(const vector<vector<string>> &spec, mips_mem_h mem, mips_cpu_h cpu){
	// Iterate through spec and execute tests accordingly
	uint32_t type;
	string func;
	int testId;

	for (uint i=0; i<spec.size();i++){
		if (spec[i][0]=="Rtype"||spec[i][0]=="RTtype"||spec[i][0]=="Jtype"||spec[i][0]=="Itype"){
			// Ignore header rows
			cout << "Ignore " << spec[i][0] << " header row." << endl;
			continue;
		}
		type = s_to_ui(spec[i][0]);
		func = spec[i][1];
		// Reset results - [i][2] contains the message
		result_set results(1,spec[i][2]);
		// Begin test
		testId = mips_test_begin_test(func.c_str());

		mips_cpu_reset(cpu);

		switch(type){
		case instr_R_type:
			test_r_type(spec[i],results, mem, cpu);
			break;
		case instr_RT_type:
			test_rt_type(spec[i],results,mem,cpu);
			break;
		case instr_J_type:
			test_j_type(spec[i],results,mem,cpu);
			break;
		case instr_I_type:
			test_i_type(spec[i],results,mem,cpu);
			break;
		}
		if(!results.passed){cout << "Test " << testId << ", " << func <<". Testing " << results.msg << ". Result:" << results.passed << endl;}
		mips_test_end_test(testId, results.passed, results.msg.c_str());
	}
}

// R-type Functions
void test_r_type(const vector<string> &row,result_set &results, mips_mem_h mem, mips_cpu_h cpu){
	// Initialise.
	uint32_t s, s_val,t, t_val, h,d,ans,ans2,loc,d_val;
	string a_loc, func;
	// Get parameter values from row
	func = row[1];
	s = s_to_ui(row[3]);
	t = s_to_ui(row[4]);
	d = s_to_ui(row[5]);
	h = s_to_ui(row[6]);
	s_val = s_to_ui(row[7]);
	t_val = s_to_ui(row[8]);
	d_val = s_to_ui(row[9]);
	a_loc = row[10];
	ans = s_to_ui(row[11]);
	ans2 = s_to_ui(row[12]);
	results.ans = s_to_ui(row[11]);
	results.ans2 = s_to_ui(row[12]);
	loc = s_to_ui(row[13]);
	// Write values to appropriate registers
	mips_cpu_set_register(cpu,d,d_val);
	mips_cpu_set_register(cpu,s,s_val);
	mips_cpu_set_register(cpu,t,t_val);
	// Get value of the pc
	mips_cpu_set_pc(cpu, loc);

	// Create a model state with the correct dest value
	model_state model(cpu, results.ans, d, loc+4);
	if (func=="SLT"){
		//cout << d << ":" << d_val << endl;
		//cout << ans << endl;
	}
	if (a_loc=="j"){
		model_state model(cpu,0,0,results.ans);
	}
	else if (a_loc=="jal"){
		model_state model(cpu,loc+8,d,results.ans);
	}
	// Make bitstream
	vector<uint32_t> p {s,t,d,h};
	uint32_t bits = test_construct_bitstream(func, instr_R_type, p);

	// Write bitstream to memory location
	mips_error err = mips_mem_write(mem, loc, 4, (uint8_t*)&bits);
	if (err!=mips_Success){
		cout << "Write to memory error." << endl;
		cout << "loc: " << loc;
		cout <<"0x";cout << hex << __builtin_bswap32(bits); cout << endl;
	}
	//if (mips_test_check_err(err, results)){return;}
	// Step cpu
	err = mips_cpu_step(cpu);

	// Check for answer in specified location

	if (a_loc=="err"){
		if(err!=results.ans){
			results.passed=0;
			fprintf(stdout,"Was expecting err = 0x%x, got 0x%x\n",results.ans, err);
		}
		return;
	}
	else if (a_loc=="hilo"){
		//cout << "This is a MULT or a DIV." << endl;
		// Write instruction to memory
		s=0;t=0;d=2;h=0;
		vector<uint32_t> p = {s,t,d,h};
		bits = test_construct_bitstream("MFHI",instr_R_type,p);
		mips_mem_write(mem, 4, 4, (uint8_t*)&bits);
		// Step the cpu
		mips_cpu_step(cpu);
		// Get the values in the dest reg
		uint32_t hi; uint32_t lo;
		mips_cpu_get_register(cpu,2,&hi);
		// Write instruction to memory
		bits = test_construct_bitstream("MFLO",instr_R_type,p);
		mips_mem_write(mem, 8,4, (uint8_t*)&bits);
		// Step the CPU
		mips_cpu_step(cpu);
		mips_cpu_get_register(cpu,2,&lo);
		//
		//cout << "hi:" << hi << " ans:" << ans << endl;
		//cout << "lo:" << lo << " ans2:" << ans2 << endl;
		if (ans!=hi){
			results.passed = 0;
			fprintf(stdout,"Was expecting hi = 0x%x, got 0x%x\n",results.ans, lo);
		}
		if (ans2!=lo){
			results.passed = 0;
			fprintf(stdout,"Was expecting lo = 0x%x, got 0x%x\n",results.ans2, hi);
		}
	}
	else if (a_loc=="reg"){
		compare_model(cpu, model, results);
		return;
	}
	return;
}
// END OF R-type Functions
// RT-type Functions
void test_rt_type(const vector<string> &row, result_set &results, mips_mem_h mem, mips_cpu_h cpu){
	// Initialise
	uint32_t s,i,s_val,loc;
	int32_t ans;
	string func, a_loc;
	func = row[1];
	s = s_to_ui(row[3]);
	i = s_to_ui(row[4]);
	s_val = s_to_ui(row[5]);
	a_loc = row[6];
	ans = s_to_i(row[7]);
	loc = s_to_ui(row[8]);

	// Write values to appropriate registers
	mips_cpu_set_register(cpu,s,s_val);

	// Set the value of the pc
	mips_cpu_set_pc(cpu, loc);

	// Create a model state with the correct dest value
	model_state model(cpu,0,0,(int32_t)loc+ans);

	// Make bitstream
	vector<uint32_t> p {s,i};
	uint32_t bits = test_construct_bitstream(func,instr_RT_type,p);

	// Write bitstream to memory location
	mips_mem_write(mem, loc, 4, (uint8_t*)&bits);

	// Step the cpu
	mips_error err = mips_cpu_step(cpu);

	// Compare to model
	if (a_loc =="err"){
		if(err!=ans){
			results.passed=0;
		}
		return;
	}
	compare_model(cpu, model, results);
}
// END OF RT-type functions
// J-type functions
void test_j_type(const vector<string> &row, result_set &results, mips_mem_h mem, mips_cpu_h cpu){
	// Initialise
	uint32_t j,ans,loc;
	string func, a_loc;
	model_state model;
	func = row[1];
	j = s_to_ui(row[3]);
	a_loc = row[4];
	ans = s_to_ui(row[5]);
	loc = s_to_ui(row[6]);
	// Set the value of the pc (0)
	mips_cpu_set_pc(cpu, loc);

	// Create a model state with correct values
	if (a_loc=="jal"){
		model_state model(cpu,31,loc+8,ans);
	}
	else{
		model_state model(cpu,0,0,ans);
	}

	// Make bitsteam
	vector<uint32_t> p {j};
	uint32_t bits = test_construct_bitstream(func,instr_J_type,p);
	// Write bitstream to memory
	mips_mem_write(mem, loc,4,(uint8_t*)&bits);

	// Step the cpu
	mips_error err = mips_cpu_step(cpu);

	// Compare the model
	if (a_loc=="err"){
		if(err!=ans){
			results.passed=0;
		}
		return;
	}
	compare_model(cpu,model,results);
}
// END OF J-type Functions
// I-type Functions
void test_i_type(const vector<string> &row, result_set &results, mips_mem_h mem, mips_cpu_h cpu){
	// initialise
	uint32_t s,t,i,s_val, t_val, ans,loc, mem_loc;
	string func, a_loc;
	func = row[1];
	s = s_to_ui(row[3]);
	t = s_to_ui(row[4]);
	i = s_to_ui(row[5]);
	s_val = s_to_ui(row[6]);
	t_val = s_to_ui(row[7]);
	a_loc = row[8];
	ans = s_to_ui(row[9]);
	mem_loc = s_to_ui(row[10]);
	loc = s_to_ui(row[11]);

	model_state model;
	// Set the value of the pc
	mips_cpu_set_pc(cpu,loc);

	// Set the value of the registers
	mips_cpu_set_register(cpu,s,s_val);
	mips_cpu_set_register(cpu,t,t_val);

	// Create a model state with the correct values
	if (a_loc=="regs"){
		model_state model(cpu, t, ans,loc+4);
	}
	else if (a_loc=="j"){
		model_state model(cpu,0,0,ans);
	}

	// make bitstream
	vector<uint32_t> p{s,t,i};
	uint32_t bits = test_construct_bitstream(func,instr_I_type, p);

	// Write bitstream to memory location
	mips_mem_write(mem, loc,4,(uint8_t*)&bits);

	// Step the cpu
	mips_error err = mips_cpu_step(cpu);

	if (a_loc=="err"){
		if(err!=ans){
			results.passed=0;
		}
		return;
	}

	else if (a_loc=="mem"){
		// Check mem loc for correct answer
		uint32_t tmp;
		mips_mem_read(mem, mem_loc, 4, (uint8_t*)&tmp);
		tmp = __builtin_bswap32(tmp);
		if (tmp!=ans){
			results.passed=0;
		}
	}

	else {
		compare_model(cpu,model,results);
	}
}
// END OF I-type functions

// Could split into different functions which are more specific
// Would have to define these categories myself, but for example: R-type normal, R-type branch, R-type MULT/DIV; I-type normal, I-type branch, I-type memory; J-type;
// Use a_loc to define the type -> perhaps 0x10 normal R-type, 0x11 R-type branch etc, and this would define which bits of the .csv are used.
// 0x30 I-type normal, 0x31 I-type branch, 0x32 I-type memory
// And have a function which tests normal (r-type and i-type), a function which tests branches (R, RT, J and I type), a function which tests memory (I-type).
// Could have helper functions which set the variables according to which type i.e. for I-type it will set s, t and i, and then construct the bitstream using I-type
// This would mean a function for each type of testing. At the moment we have a function for each type of instruction, which means lots of if clauses to figure out
// how to test that particular function

// This would be selected depending on the first half byte of the first item in the row
// This is just normal operations which means one answer that appears in the dest reg OR an error
// Can confirm, this looks much nicer than the previous way

bool not_digit(string line){
    char * p;
    strtol(line.c_str(), &p, 10);
    if(*p){
    	return 1;
    }
    return 0;
}

//! This will replace run_spec when it's child functions are ready
void run_spec2(const vector<vector<string>> &spec, mips_mem_h mem, mips_cpu_h cpu){
	// Iterate through spec and execute tests accordingly
	// from now one func will be in row 2 and message will be in row 1
	uint32_t type;
	string func = spec[0][2];
	int testId;
	for (uint i=0;i<spec.size();i++){
		if (spec[i][0]=="Rnorm" || spec[i][0]=="Rmdiv"){
			// Ignore header rows
			cout << "Ignore " << spec[i][0] << " header row." << endl;
			continue;
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

		switch(type){
		case test_Normal: // 0x1
			test_normal_functions(spec[i],results,mem,cpu);
			break;
		case test_Branch: //0x2
			test_branch_functions(spec[i],results,mem,cpu);
			break;
		case test_MemWrite: //0x3
			break;
		case test_MemRead: //0x4
			break;
		case test_MTMF: //0x5
			break;
		case test_MultDiv: //0x6
			break;
		default:
			cout << "Didn't recognise type " << type << "skipping test." << endl;
			continue;
		}
		if(!results.passed){
			cout << "Test " << testId << ", " << func <<". Testing " << results.msg << ". Result:" << results.passed << endl;
		}
		mips_test_end_test(testId, results.passed, results.msg.c_str());
	}
}

//! \todo finish this function, write .csv for testing normal functions
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
		params = {s,dest,i};
		instruction_bits = test_construct_bitstream(func, instr_I_type, params);
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
	// Set d_val to non-zero because it should be overwritten
	err = mips_cpu_set_register(cpu, dest, 11);
	// Write bitstream to memory
	err = mips_mem_write(mem, 0, 4,(uint8_t*)&instruction_bits);
	// Create model
	model_state model(cpu,dest,ans,4);
	// Step CPU
	err = mips_cpu_step(cpu);
	if (exp_err||err){
		if (err != exp_err){
			results.passed = 0;
			cout << hex << "Incorrect error message " << err << " was expecting " << exp_err << endl;
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
	uint32_t s,t,dest,d,h,t_val,s_val,target,loc,link,exp_err,branch;
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
		s = s_to_ui(row[3]); // s contains the value to jump to
		dest = s_to_ui(row[4]); // t_reg - want to test == 0 corner case
		d = s_to_ui(row[5]); // only used in error msg test
		h = s_to_ui(row[6]);
		target = s_to_ui(row[7]); // s_val
		link = s_to_ui(row[8]);
		exp_err = s_to_ui(row[9]);
		loc = 0; // For R type, we only jump forwards, so write to 0 location
		// Create r-type bitstream (d,h are usually 0, included to test corner case)
		params = {s,dest,d,h};
		instruction_bits = test_construct_bitstream(func,instr_R_type,params);
		err = mips_cpu_set_register(cpu, s, target);
		break;
	case instr_RT_type:
		// This would be RT-type functions, these compare s and t
		dest = 31;
		s = s_to_ui(row[3]);
		i = s_to_i(row[4]); // sign extend i
		s_val = s_to_ui(row[5]);
		loc = s_to_ui(row[6]);
		branch = s_to_ui(row[7]);
		target = (int32_t)loc + (i<<2);
		params = {s,s_to_ui(row[4])};
		instruction_bits = test_construct_bitstream(func, instr_RT_type, params);
		err = mips_cpu_set_register(cpu,s,s_val);
		break;
	case instr_I_type:
		// This would be I-type functions, can't link
		dest=0;link=0;
		// branch depends on s and t and s_val and t_val
		s = s_to_ui(row[3]);
		t = s_to_ui(row[4]);
		i = s_to_i(row[5]); // sign extend i
		s_val = s_to_ui(row[6]);
		t_val = s_to_ui(row[7]);
		loc = s_to_ui(row[8]);
		branch = s_to_ui(row[9]);
		target = (int32_t)loc + (i<<2);
		// create I type bitstream
		params = {s,t,s_to_ui(row[5])};
		instruction_bits = test_construct_bitstream(func, instr_I_type, params);
		// Set s_val and t_val
		err = mips_cpu_set_register(cpu,s,s_val);
		err = mips_cpu_set_register(cpu,t,t_val);
		break;
	case instr_J_type:
		dest = 31;
		target = s_to_ui(row[3]);
		// create j-type bitstream
		params = {target};
		instruction_bits = test_construct_bitstream(func, instr_J_type, params);
		break;
	default:
		cout << "Strange instruction type code ended up in branch handler, shouldn't be here." << endl;
		results.passed = 0;
		return;
	}

	if (exp_err){
		err = mips_cpu_step(cpu);
		if (err!=exp_err){
			results.passed = 0;
			cout << hex << "Incorrect error message " << err << " was expecting " << exp_err << endl;
		}
		return;
	}
	if (link){
		link = loc + 8;
		err = mips_cpu_set_register(cpu,dest,1);
	}
	// Write jump/branch instruction to loc
	err = mips_mem_write(mem, loc, 4, (uint8_t*)&instruction_bits);

	//! Create delay slot instruction.
	uint32_t d_reg,d_ans;
	d_ans = 5;
	d_reg = 6;
	// Set registers
	err = mips_cpu_set_register(cpu,4,3);
	err = mips_cpu_set_register(cpu,5,2);
	params = {4,5,d_reg,0};
	instruction_bits = test_construct_bitstream("ADDU",instr_R_type,params);
	// Write add instruction to delay slot location
	err = mips_mem_write(mem, loc+4, 4, (uint8_t*)&instruction_bits);
	err = mips_cpu_step(cpu);
	// After the first step, nothing will have changed except the PC must be +4 and reg[dest==link address]
	model_state model(cpu,dest,link,loc+4);
	// If the cpu state doesn't match the model, return, no further testing.
	if (compare_model(cpu,model,results)){
		cout << "Failed at first step of branch instruction." << endl;
		return;
	}
	err = mips_cpu_step(cpu);
	// After the second step, the delay slot instruction must execute (always do a simple add for this)
	model.regs[d_reg] = d_ans;
	// if not meant to branch, check that delay slot instruction executed, and that pc = loc + 8
	if (!branch){
		model.pc = loc+8;
		compare_model(cpu,model,results);
		return;
	}

	// and PC must be = target. Also reg[dest] must equal PC+8, but we did that earlier
	model.pc = target;

	if (compare_model(cpu,model,results)){
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
	model.pc = target + 4;
	model.regs[j_reg] = j_ans;
	if (compare_model(cpu,model,results)){
		cout << "Failed at third step of branch instruction." << endl;
	}

	return;
}

//! These are quite simple to test, need to write a specified value to a memory location using the cpu
//! then check the memory for correct written value. These are all I-type instructions.
void test_write_memory_functions(const vector<string> &row, result_set &results, mips_mem_h mem, mips_cpu_h cpu){

}

//! These are quite simple to test. Write specified value to memory location using mips_mem_write
//! Then check the memory for correct value using cpu function. Again, all I-type functions
void test_read_memory_functions(const vector<string> &row, result_set &results, mips_mem_h mem, mips_cpu_h cpu){

}

//! These functions require their sister functions to work in order to be tested properly
void test_mtmf_functions(const vector<string> &row, result_set &results, mips_mem_h mem, mips_cpu_h cpu){

}
//! These are fairly complicated to test, since they require multiple steps through the CPU.
//! Require MFHI MFLO to be working in order to test as well.
void test_multdiv_functions(const vector<string> &row, result_set &results, mips_mem_h mem, mips_cpu_h cpu){
	cout << "Got to multdiv functions, not ready yet, fail test." << endl;
	results.passed = 0;
	return;

}



