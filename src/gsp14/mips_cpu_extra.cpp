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


/*
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
*/

	//cout << test_add(mem,cpu).msg << endl;

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


/*
 * Parse through the file, executing the instructions listed on the MIPS
 * Get desired output from the test_spec
 * Set model_state to desired output
 */
/*
 *
	string func;
	uint32_t vals,valt,dest,shift,out,err;
for (auto v : terms){
			std::cout << v << "\n";
		}
		func = terms[0];
		vals = strtol(terms[1].c_str(),NULL,0);
		valt = strtol(terms[2].c_str(),NULL,0);
		dest = strtol(terms[3].c_str(),NULL,0);
		shift = strtol(terms[4].c_str(),NULL,0);
		out = strtol(terms[2].c_str(),NULL,0);
		err = strtol(terms[5].c_str(),NULL,0);
		cout << hex << "vals: " <<vals << " valt: " << valt << " dest: " << dest << " shift: " << shift << " out: " << out;
		cout << " err: " << err << endl;
		cin.get();
*/

/*
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
*/


//xa[0]={0x00, 0x11, 0x00, 0xFF};

/*
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
*/
