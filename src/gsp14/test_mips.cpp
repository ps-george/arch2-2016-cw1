/*
 * test_mips.cpp
 *
 *  Created on: 14 Oct 2016
 *      Author: big-g
 */

//#include "test_mips_functions.cpp"

#include <string>
#include "string.h"
#include <iostream>
#include <map>
#include <sstream>
#include "test_mips_gsp14.h"
#include "shared.cpp"

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

result_set test_step(mips_mem_h, mips_cpu_h);
result_set test_add(mips_mem_h, mips_cpu_h);
result_set test_addi(mips_mem_h, mips_cpu_h);
result_set test_addiu(mips_mem_h, mips_cpu_h);

/*!
 * Defined a function pointer which takes in an int and returns a result set.
 * @param
 * @return
 */
typedef result_set (*pFunc)(mips_mem_h, mips_cpu_h);

map<int, string> nameTestMap = {
		{0, "<internal>"}
		/*{1,"ADD"},
		{2,"ADDI"},
		{3,"ADDIU"},
		{4,"ADDU"}
		*/
};

map<int, pFunc> testMap = {
		{0, &test_step}
		/*{1,&test_add},
		{2,&test_addi},
		{3,&test_addiu}*/
};


void set_debug_level(int argc, char* argv[],mips_cpu_h cpu);

int main(int argc, char* argv[])
{
	/*! Print out arguments */
    int i;
    for(i=0; i<argc; ++i)
    {
    	printf("Argument %d : %s\n", i, argv[i]);
    }

    /*! Reverse map of opcodes for easy creation of opcodes. */
    for (auto const &i: opcode_to_str) {
        	str_to_opcode[i.second] = i.first;
        }

	/*! Create memory */
	mips_mem_h mem = mips_mem_create_ram(4096);

	/*! Create cpu */
	mips_cpu_h cpu = mips_cpu_create(mem);

	/*! Set debug level */
	set_debug_level(argc,argv,cpu);
	mips_cpu_step(cpu);
	/*! Prepare for tests */
	mips_test_begin_suite();

	string testName;
	int testId = 0;
	result_set results;
	int NUM_TESTS = 1;
	/*! \todo Loop which runs through all tests in an array */
	for (int i = 0;i<NUM_TESTS;i++)
	{
		testName = nameTestMap[i];

		/*! Begin running test "string" */
		testId=mips_test_begin_test(testName.c_str());
		cout << "Test id is: " << testId << endl;

		/*! Run tests and write results to result_set object */
		results = testMap[i](mem,cpu);
		fprintf(stdout,"Test %d done", testId);
		/*! mips_test_end_test will get results from result_set */
		mips_test_end_test(testId, results.passed, results.msg.c_str());

	}
	mips_test_end_suite();
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

result_set test_step(mips_mem_h mem, mips_cpu_h cpu){
	// Get the pc value

	// Get value at memory location

	// Decode the value of the memory location

	//
	result_set results(0);
	mips_error err = mips_cpu_step(cpu);
	uint32_t pc;
	mips_cpu_get_pc(cpu,&pc);
	//mips_mem_read(mem,pc,4, );

	if (err == mips_Success){
		results.passed = 1;
	}
	results.msg = err;
	return results;
}

result_set test_add(mips_mem_h, mips_cpu_h){
	result_set results(0, "Yes");
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
