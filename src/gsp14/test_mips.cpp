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


map<string, uint32_t> ij_to_op;

struct model_state{
	uint32_t pc;
	uint32_t regs[32];
	model_state(mips_cpu_h cpu, uint32_t val, uint32_t dest, uint32_t Npc);

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

struct result_set{
	int passed;
	stringstream msg;

	result_set();
	result_set(int passed);
	result_set(int passed, string msg_in);

};
result_set::result_set(int passed_in, string msg_in){
	msg << msg_in;
	passed = passed_in;
}
result_set::result_set(int passed_in){
	msg << "";
	passed = passed_in;
}

void compare_model(mips_cpu_h cpu, model_state model, result_set &results){
	mips_error err;
	uint32_t tmp, pc;
	for (unsigned i=0;i<32;i++){
		err = mips_cpu_get_register(cpu, i, &tmp);
		if (err!=mips_Success){
			results.msg << "Error reading from register " << i << endl;
			results.passed=0;
			return;
		}
		if (model.regs[i]!=tmp) {
			results.msg << "Register" << i << "is value" << tmp << ", should be value "
			<< model.regs[i] << endl;
			results.passed=0;
		}
	}
	err = mips_cpu_get_pc(cpu, &pc);
	if (mips_test_check_err(err, results)){return;}
	if (pc!=model.pc){
		results.msg << "PC is " << pc << ". Should be " << model.pc << "." << endl;
		results.passed=0;
	}
	return;
}


/*!
 * Take in a list of params and function name and construct a bitstream for MIPS from it.
 * @param params - s,t,d,h for R-type; s,i for RT-type; j for J type; s,t,i for I type
 * @param func - string containing the function name
 * @return
 */
uint32_t test_construct_bitstream(string func,const vector<uint32_t> &params){
	uint32_t bitstream=0x0;
	uint32_t s,t,d,h,i,j;
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

int mips_test_check_err(mips_error err, result_set &results){
	if (err!=mips_Success){
		results.passed = 0;
		results.msg << "Mips error code: " << err;
		return 1;
	}
	return 0;
}

void test_r_type(const vector<string> &row,result_set &results, mips_mem_h mem, mips_cpu_h cpu){
	// Initialise.
	uint32_t s, s_val,t, t_val, h,d,ans,ans2;
	string ans_loc, func;
	// Get parameter values from row
	func = row[1];
	s = s_to_ui(row[2]);
	t = s_to_ui(row[3]);
	d = s_to_ui(row[4]);
	h = s_to_ui(row[5]);
	s_val = s_to_ui(row[6]);
	t_val = s_to_ui(row[7]);
	ans_loc = row[8];
	ans = s_to_ui(row[9]);
	ans2 = s_to_ui(row[10]);
	// Write values to appropriate registers
	mips_cpu_set_register(cpu,s,s_val);
	mips_cpu_set_register(cpu,t,t_val);

	// Get value of the pc
	uint32_t pc;
	mips_cpu_get_pc(cpu, &pc);

	// Create a model state with the correct dest value
	model_state model(cpu, ans, d,pc+4);

	// Make bitstream
	vector<uint32_t> p = {s,t,d,h};
	uint32_t bits = test_construct_bitstream(func, p);
	cout << "0x";cout << hex << __builtin_bswap32(bits);cout << endl;

	// Write bitstream to memory location
	mips_error err = mips_mem_write(mem, 0, 4, (uint8_t*)&bits);
	if (mips_test_check_err(err, results)){return;}
	// Step cpu
	err = mips_cpu_step(cpu);

	// Check for answer in specified location
	if (row[8]=="reg"){
		compare_model(cpu, model, results);
		return;
	}
	else if (row[8]=="err"){
		if (err!=ans){
			results.passed = 0;
			results.msg << "Incorrect error message: " << err;
			return;
		}
	}
	else if (row[8]=="hilo"){
		// Write instructions to memory, specify two different dest regs
		s=0;t=0;d=2;h=0;
		vector<uint32_t> p = {s,t,d,h};
		bits = test_construct_bitstream("MFHI",p);
		mips_mem_write(mem, 4,4, (uint8_t*)&bits);
		p[2] = 3;
		bits = test_construct_bitstream("MFLO",p);
		mips_mem_write(mem, 8,4, (uint8_t*)&bits);
		// Step the CPU twice
		mips_cpu_step(cpu);
		mips_cpu_step(cpu);
		// Get the values in the dest reg
		uint32_t hi; uint32_t lo;
		mips_cpu_get_register(cpu,2,&hi);
		mips_cpu_get_register(cpu,3,&lo);
		//
		if (ans!=hi){
			results.passed = 0;
			results.msg << "hi is wrong." << endl;
		}
		if (ans2!=lo){
			results.passed = 0;
			results.msg << "lo is wrong." << endl;
		}
	}
	return;
}

void run_spec(const vector<vector<string>> &spec, mips_mem_h mem, mips_cpu_h cpu){
	// Iterate through spec and execute tests accordingly
	uint32_t type;
	string func = spec[0][1];

	result_set results(1,"");
	int testId=mips_test_begin_test(func.c_str());

	for (uint i=0; i<spec.size();i++){
		type = strtol(spec[i][0].c_str(),NULL,8);
		if (func!=spec[i][1]){
			// End the test
			cout << results.msg.str() << endl;
			mips_test_end_test(testId, results.passed, results.msg.str().c_str());
			// Reset the results
			result_set results(1,"");
			func = spec[i][1];
			testId = mips_test_begin_test(func.c_str());
		}

		mips_cpu_reset(cpu);

		switch(type){
		case instr_R_type:
			test_r_type(spec[i],results, mem, cpu);
			break;
		case instr_RT_type:
			//
			break;
		case instr_J_type:
			//
			break;
		case instr_I_type:
			//
			break;
		}

	}
}

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

	// Set debug level
	set_debug_level(argc,argv,cpu);

	// Prepare for tests
	mips_test_begin_suite();

	// Test that all 32 registers exist and contain uint32_t

	// Read test_spec into vector of strings
	vector<vector<string> > spec1;
	read_test_spec("test_spec.csv", spec1);

	// Execute test spec
	run_spec(spec1, mem,cpu);
		// Loop through all the vectors




	return 0;
}

// Function which takes in a string naming the test file, tests all the cases in the file
int read_test_spec(string filename, vector<vector<string> > &spec){
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
		/* For debugging
		for (auto v : spec[i]){
			cout << v << "\n";
		}
		cin.get();
		*/
		i++;
	}
	test_file.close();
	return 0;
}



uint32_t test_construct_R_bitstream(std::string rfunc, uint32_t src1, uint32_t src2, uint32_t dest, uint32_t shift_amt){
	uint32_t bitstream=0x00000000, fn_code = 0;
	// Find the fn code of the string
	fn_code = r_to_op.at(rfunc);
	// Find the rest of the bitstream
	bitstream = bitstream | (src1 << 21) | (src2 << 16) | (dest << 11) | (shift_amt<<6) | fn_code;
	return bitstream;
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

/*!
 * Writes an R function into memory at location loc.
 * @param mem - Memory handler (needed because don't know internals of CPU to access memory)
 * @param loc - Need to ensure PC==loc to ensure the instruction is executed.
 * @param func - String of R func to write to memory
 * @param reg1 - index of reg1
 * @param reg2 - index of reg2
 * @param shift_amt - shift amount (if shifting)
 * @param dest - index of destination register
 * @return
 */
mips_error write_R_func(
		mips_mem_h mem,
		uint32_t loc,
		string func,
		uint32_t reg1,
		uint32_t reg2,
		uint32_t shift_amt,
		uint32_t dest){
	mips_error err = mips_Success;
	uint32_t w = test_construct_R_bitstream(func,reg1,reg2,dest,shift_amt);
	w = __builtin_bswap32(w);
	// Write instruction to memory at specified location
	err = mips_mem_write(mem, loc, 4, (uint8_t*)&w);
	return err;
}

