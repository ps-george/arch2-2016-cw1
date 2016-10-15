/*
 * test_mips.cpp
 *
 *  Created on: 14 Oct 2016
 *      Author: big-g
 */

//#include "test_mips_functions.cpp"
int NUM_TESTS = 3;

#include <string>
#include "string.h"
#include <iostream>
#include <map>
#include <sstream>
#include "test_mips_gsp14.h"

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
typedef result_set (*pFunc)(mips_mem_h, mips_cpu_h);

map<int, string> nameTestMap = {
	{1,"ADD"},
	{2,"ADDI"},
	{3,"ADDIU"},
	{4,"ADDU"}
};

map<string, pFunc> testMap = {
		{"ADD",&test_add},
		{"ADDI",&test_addi},
		{"ADDIU",&test_addiu}
};


int main(int argc, char* argv[])
{
    int i;
    for(i=0; i<argc; ++i)
    {   printf("Argument %d : %s\n", i, argv[i]);
    }
    //cout << "All arguments read." << endl;
	/*! Create memory */
	mips_mem_h mem = mips_mem_create_ram(4096);
	//cout << "Memory created." << endl;
	/*! Create cpu */
	mips_cpu_h cpu = mips_cpu_create(mem);
	//cout << "CPU created." << endl;
	/*! Set debug level */
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

	/*! Prepare for tests */
	mips_test_begin_suite();

	string testName;
	int testId = 0;
	result_set results;
	/*! \todo Loop which runs through all tests in an array */
	for (int i = 1;i<NUM_TESTS+1;i++)
	{
		testName = nameTestMap[i];

		/*! Begin running test "string" */
		testId=mips_test_begin_test(testName.c_str());
		cout << "Test id is: " << testId << endl;

		/*! Run tests and write results to result_set object */
		results = testMap[testName](mem,cpu);
		fprintf(stdout,"Test %d done", testId);
		/*! mips_test_end_test will get results from result_set */
		mips_test_end_test(testId, results.passed, results.msg.c_str());

	}
	mips_test_end_suite();
	return 0;
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
