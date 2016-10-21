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
* MAP/STRUCT DEFINITIONS
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

struct model_state{
	uint32_t pc;
	uint32_t regs[32];
	model_state(mips_cpu_h cpu, uint32_t val, uint32_t dest, uint32_t Npc);
	model_state();
};

// Model based on actual state of the CPU, except with desired values inside.
model_state::model_state(mips_cpu_h cpu, uint32_t val, uint32_t dest, uint32_t Npc){
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
* END OF DEFINITIONS
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
 * @return
 */
// CONSTRUCT BIT STREAM
uint32_t test_construct_bitstream(string func,const vector<uint32_t> &params){
	uint32_t bitstream=0x0;
	uint32_t s,t,d,h,i,j;
	//cout << func << endl;
	switch(params[0]){
	case instr_R_type: // R-type
		s = params[1];
		t = params[2];
		d = params[3];
		h = params[4];
		bitstream = 0x0 |  (s << 21) | (t << 16) | (d << 11) | (h<<6) | r_to_op.at(func);
		break;
	case instr_RT_type: // RT-type
		s = params[1];
		i = params[2];
		bitstream = 0x04000000 | (s << 21) | (rt_to_op.at(func) << 16)| i;
		break;
	case instr_J_type: // J type
		j = params[1];
		bitstream = ij_to_op.at(func)<<26|j;
		break;
	case instr_I_type: //I-type
		s = params[1];
		t = params[2];
		i = params[3];
		bitstream = ij_to_op.at(func)<<26|(s<<21)|(t<<16)|i;
		break;
	}
	bitstream = __builtin_bswap32(bitstream);
	return bitstream;
}
// END OF CONSTRUCT bitstream
// COMPARE MODEL
void compare_model(mips_cpu_h cpu, model_state model, result_set &results){
	mips_error err;
	uint32_t tmp, pc;
	for (unsigned i=0;i<32;i++){
		err = mips_cpu_get_register(cpu, i, &tmp);
		if (err!=mips_Success){
			//results.msg << "Error reading from register " << i << endl;
			results.passed=0;
			return;
		}
		if (model.regs[i]!=tmp) {
			//results.msg << "Register" << i << "is value" << tmp << ", should be value "
			//<< model.regs[i] << endl;
			results.passed=0;
		}
	}
	err = mips_cpu_get_pc(cpu, &pc);
	if (mips_test_check_err(err, results)){return;}
	if (pc!=model.pc){
		//results.msg << "PC is " << pc << ". Should be " << model.pc << "." << endl;
		results.passed=0;
		fprintf(stdout,"Was expecting answers 0x%x,0x%x\n",results.ans, results.ans2);
	}
	return;
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
	string func = spec[0][1];
	int testId;

	for (uint i=0; i<spec.size();i++){
		if (spec[i][0]=="Rtype"||spec[i][0]=="RTtype"||spec[i][0]=="Jtype"||spec[i][0]=="Itype"){
			// Ignore header rows
			cout << "ignore " << spec[i][0] << endl;
			continue;
		}
		type = s_to_ui(spec[i][0]);
		// End the test

		// Reset the results
		func = spec[i][1];
		result_set results(1,spec[i][2]);
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
// Could split into different functions which are more specific
// Would have to define these categories myself, but for example: R-type normal, R-type branch, R-type MULT/DIV; I-type normal, I-type branch, I-type memory; J-type;
// Use a_loc to define the type -> perhaps 0x10 normal R-type, 0x11 R-type branch etc, and this would define which bits of the .csv are used.
// 0x20 I-type normal, 0x21 I-type branch, 0x22 I-type memory
// And have a function which tests normal (r-type and i-type), a function which tests branches (R, RT, J and I type), a function which tests memory (I-type).
// Could have helper functions which set the variables according to which type i.e. for I-type it will set s, t and i, and then construct the bitstream using I-type
// This would mean a function for each type of testing. At the moment we have a function for each type of instruction, which means lots of if clauses to figure out
// how to test that particular function
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
	vector<uint32_t> p {instr_R_type,s,t,d,h};
	uint32_t bits = test_construct_bitstream(func, p);

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
		vector<uint32_t> p = {instr_R_type,s,t,d,h};
		bits = test_construct_bitstream("MFHI",p);
		mips_mem_write(mem, 4, 4, (uint8_t*)&bits);
		// Step the cpu
		mips_cpu_step(cpu);
		// Get the values in the dest reg
		uint32_t hi; uint32_t lo;
		mips_cpu_get_register(cpu,2,&hi);
		// Write instruction to memory
		bits = test_construct_bitstream("MFLO",p);
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
	vector<uint32_t> p {instr_RT_type,s,i};
	uint32_t bits = test_construct_bitstream(func,p);

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
	vector<uint32_t> p {instr_J_type,j};
	uint32_t bits = test_construct_bitstream(func,p);
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
	vector<uint32_t> p{instr_I_type, s,t,i};
	uint32_t bits = test_construct_bitstream(func, p);

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
