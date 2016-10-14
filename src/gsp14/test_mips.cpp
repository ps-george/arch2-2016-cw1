/*
 * test_mips.cpp
 *
 *  Created on: 14 Oct 2016
 *      Author: big-g
 */

//#include "test_mips_functions.cpp"
int NUM_TESTS = 4;

#include <string>
#include <iostream>
#include <map>
#include <sstream>
#include "test_mips_gsp14.h"

using namespace std;

struct result_set{
	int passed;
	char *msg;
};

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


int main(){
	/*! Create memory */
	mips_mem_h mem = mips_mem_create_ram(4096);

	/*! Create cpu */
	mips_cpu_h cpu = mips_cpu_create(mem);

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

		/*! mips_test_end_test will get results from result_set */
		mips_test_end_test(testId, results.passed, results.msg);
	}
	mips_test_end_suite();
	return 0;
}


result_set test_add(mips_mem_h, mips_cpu_h){
	result_set results;
	results.passed = 0;


	return results;
}

result_set test_addi(mips_mem_h, mips_cpu_h){
	result_set results;
	return results;
}

result_set test_addiu(mips_mem_h, mips_cpu_h){
	result_set results;
	return results;
}
