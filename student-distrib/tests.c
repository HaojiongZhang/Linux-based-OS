#include "tests.h"
#include "x86_desc.h"
#include "i8259.h"
#include "rtc_handler.h"
#include "keyboard.h"
#include "fileSystem.h"
#include "cp3_syscall.h"

#define PASS 1
#define FAIL 0
// static int fd; 

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test - Example
 * 
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i){
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}

	return result;
}

/*
Descriptor: this function is used to test exception 0
should return exception
input: none
return: none*/
void divide_by_zero(){
	int i;
	int j;
	int k;
	i = 0;
	j = 10;
	k = j/i;
	
	return;
}


/*
Descriptor: these funcs test edge cases at memory locations
should all cause exception page fault
input: none
return: none*/
void deref_invalid_address_1(){
	unsigned char* i;
	unsigned char j;
	i = (unsigned char*)0xB7FFF;
	j = *i;
}
void deref_invalid_address_2(){
	unsigned char* i;
	unsigned char j;
	i = (unsigned char*)0xB9000;
	j = *i;
}
void deref_invalid_address_3(){
	unsigned char* i;
	unsigned char j;
	i = (unsigned char*)0x3FFFFF;
	j = *i;
}
void deref_invalid_address_4(){
	unsigned char* i;
	unsigned char j;
	i = (unsigned char*)0x800000;
	j = *i;
}

void deref_invalid_null(){
	int* i;
	int j;
	i = NULL;
	j = *i;
}

/*valid test*/
/*
Descriptor: this function test deref in present mem
should not cause page fault
input: none
return: none*/
int deref_valid_address(){


	unsigned char* vms = (unsigned char*) 0xB8000;
	unsigned char* vme = (unsigned char*) 0xB9000;
	unsigned char* ks = (unsigned char*) 0x400000;
	unsigned char* ke = (unsigned char*) 0x800000;
	unsigned char a;
	unsigned char* ptr;
	//test vm
	for (ptr = vms; ptr< vme; ptr++){
		a = (*ptr);
	}
	//test kernel
	for (ptr = ks; ptr< ke; ptr++){
		a = (*ptr);
	}
	return PASS;
}

/*
Descriptor: this function test oob irq dis/enables
should not cause exceptions. disables keyboard.
input: none
return: none*/
int irq_test(){

	//OOB doesn't cause troubles
	enable_irq(16);
	disable_irq(16);
	//disable keyboard
	disable_irq(1);

	return PASS;
}


// add more tests here

/* Checkpoint 2 tests */
/*
RTC_test function
Descriptor: the function is used to test the functionality of 4 rtc functions
we print the symbol on the screen in different frequency to make sure the rtc
can work correctly
Input: NONE
Return: 0/-1*/
int rtc_test()
{
	void * buf;
	uint32_t freq;
	
	freq = 2;
	rtc_open((uint8_t*)"rtc");			//open the rtc and set the freq to 2
	while(freq <= 1024){				//for all possible freq
		// clear();						//clear the screen
		int i;
		for(i = 0; i < 80; i++){		//for every freq, print a symbol on the screen for 80 times
			rtc_read(0, buf, 4);
			putc('0');					//use modified putc
		}
		freq  *= 2;						//double the freq
		rtc_write(0, &freq, 4);			//modify the freq of the rtc
	}
	rtc_close(0);						//close the rtc
	return PASS;
}

/*
Descriptor: this function tests terminal driver
writing the read user input and specify buffer
count.
input: none
return: none*/
int32_t terminal_driver_test(){
	char buf1[128];
	char buf2[128];
	int32_t fd_read = 0;
	int32_t fd_write = 1;
	int32_t W_return;
	int32_t R_return;
	int32_t W_ret;
	int32_t R_ret;
	//test terminal write by writing the buffer read.
	printf("localhost login: ");
	R_return = terminal_read(fd_read, buf1, 128);
	printf("login as ");
	W_return = terminal_write(fd_write, buf1, R_return);
	printf("password: ");
	R_ret = terminal_read(fd_read, buf2, 128);
	clear();
	printf("login success!\n");
	printf("username: ");
	W_return = terminal_write(fd_write, buf1, R_return);
	printf("password: ");
	W_ret = terminal_write(fd_write, buf2, R_ret);
	printf("username count=%d\npassword count=%d\n", R_return - 1, R_ret - 1);
	return 0;
}
/*
Descriptor: terminal_write_test
test for writing a 256 buffer, and alt version 
with nulls in middle
inputs:none
return: number of bytes read*/
int32_t terminal_write_test(){
	char buf[256];
	int32_t fd_write = 1;
	int32_t i;
	int32_t W_return;
	for (i = 0; i < 255; i++){
		buf[i] = '6';
	}
	buf[255] = '\n';
	W_return = terminal_write(fd_write, buf, 256);
	printf("length: %d\n", W_return);
	printf("reading buffer with NULLs at middle, should not stop\n");
	for (i = 0; i < 255; i++){
		buf[i] = '6';
	}
	//creating 2 NUlls in middle of buffer
	buf[30] = NULL;
	buf[31] = NULL;
	buf[255] = '\n';
	W_return = terminal_write(fd_write, buf, 256);
	printf("length: %d", W_return);
	return W_return;
}


/*
Descriptor: terminal_null_test
test for reading to or writing from null 
return: PASS/*/
int32_t terminal_NULL_test(){
	char buf[128];
	int32_t fd_read = 0;
	int32_t fd_write = 1;
	int32_t i;
	int32_t W_return;
	int32_t R_return;
	int32_t inv1, inv2;
	R_return = terminal_read(fd_read, NULL, 128);
	W_return = terminal_write(fd_write, NULL, 256);
	//test all invalid cases should return -1
	printf("should both be -1: %d, %d\n", R_return, W_return);
	for (i = 0; i < 128; i++){
		buf[i] = '6';
	}
	inv1 = terminal_read(fd_read, buf, -1);
	inv2 = terminal_write(fd_write, buf, -1);
	printf("should both be -1: %d, %d\n", inv1, inv2);
	return PASS;
}


/*
Descriptor: this function prints buffers of length 
bytes using putc
input: buffer to be printed, length in bytes
return: none*/
void print_buf(uint8_t* buf, uint32_t length){
	int i = 0;
	for (i = 0; i < length; i++){
		putc(buf[i]);
	}
	return;
}


/*
Descriptor: this function opens file of fileName
using file_open and read the content to buf using
file_read. then it prints out the content.
input: buffer to be printed, length in bytes
inputs: none
return: none*/
int open_fileName(){
	uint8_t* fileName;
	uint8_t buf[4096];
	uint32_t length = 204; //size of frame1.txt
	int of = 0;

	dentry_t tmp_dent;

	clear();
	cpyFileDescriptor((uint32_t *)&of, 2, (uint8_t *)&tmp_dent);
	fileName = (uint8_t*)("frame1.txt");
	printf("\nOpening file %s ....\n", fileName);
	printf("\n------------------------------------------------\n");

	file_open(fileName);
	//read_dentry_by_name(fileName,&tmp_dent);
	printf("\nReading file %s ....\n", fileName);
	printf("\n------------------------------------------------\n");

	file_read(2,&buf,length);
	print_buf(buf,length);
	

	return PASS;
}

/*
Descriptor: this function opens file of fileName
using file_open and read the content to buf using
file_read. then it prints out the content. This time
it's an executable.
inputs: none
return: none*/
int open_fileName1(){
	uint8_t* fileName;
	uint8_t buf[5445];
	uint32_t length = 5445;//size of pingpong
	clear();
	
	fileName = (uint8_t*)("pingpong");
	printf("\nOpening file %s ....\n", fileName);
	printf("\n------------------------------------------------\n");

	file_open(fileName);
	//read_dentry_by_name(fileName,&tmp_dent);
	printf("\nReading file %s ....\n", fileName);
	printf("\n------------------------------------------------\n");

	file_read(0,&buf,length);
	print_buf(buf,length);
	

	return PASS;
}


/*
Descriptor: this function tries to read from a 
NULL buffer. read_data should print error statement
and return.
inputs: none
return: PASS*/
int reading_to_null_buffer(){
	uint8_t* fileName;
	dentry_t tmp_dent;

	clear();
	fileName = (uint8_t*)("frame0.txt");
	printf("\nOpening file %s ....\n", fileName);
	printf("\n------------------------------------------------\n");

	
	read_dentry_by_name(fileName,&tmp_dent);
	printf("\nReading file %s ....\n", fileName);
	printf("\n------------------------------------------------\n");


	read_data(tmp_dent.inode_num, 0, 0, 20);
	return PASS;
}


/*
Descriptor: this function tries to read using a 
file name longer than 32B. file_open should print
an error statement and return.
inputs: none
return: PASS*/
int verylongnametest(){
	uint8_t* fileName;
	
	clear();
	fileName = (uint8_t*)("verylargetextwithverylongname.txt");
	printf("\nOpening file %s ....\n", fileName);
	printf("\n------------------------------------------------\n");

	
	file_open(fileName);
	
	return PASS;
}


/*
Descriptor: this function tries to go through the
dir_entries in the bootblock and print all of them
inputs: none
return: PASS*/
int open_dirName(){
	uint8_t* dirName;
	uint8_t buf[4096];
	uint32_t length = 32; 

	clear();
	dirName = (uint8_t*)(".");
	printf("\nOpening dir %s ....\n", dirName);
	printf("\n------------------------------------------------\n");

	dir_open(dirName);

	
	printf("\nReading dir %s ....\n", dirName);
	printf("\n------------------------------------------------\n");

	//go through loop to print whole dir
	while(dir_read(0,&buf,length)==0){
		print_buf(buf,length);
		printf("\n");
	}

	return PASS;
}

/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	/*check point 1 test cases*/
	// TEST_OUTPUT("idt_test", idt_test());
	// TEST_OUTPUT("irq_test, keyboard disabled", irq_test());
	// TEST_OUTPUT("paging_test", deref_valid_address());
	// deref_invalid_address_1();
	// deref_invalid_address_2();
	// deref_invalid_address_3();
	// deref_invalid_address_4();
	// deref_invalid_null();
	// divide_by_zero();

	/*check point 2 test cases*/
	// TEST_OUTPUT("rtc_test", rtc_test());
	// terminal_driver_test();
	// terminal_write_test();
	//sys_execute((uint8_t *)("shell"));
	//open_fileName();
	// TEST_OUTPUT("terminal null_test",terminal_NULL_test());
	// TEST_OUTPUT("test open txt file",open_fileName());
	// TEST_OUTPUT("test open dir",open_dirName());
	// TEST_OUTPUT("test open verylong",verylongnametest());
	// TEST_OUTPUT("test read null",reading_to_null_buffer());
	// TEST_OUTPUT("test open exe file",open_fileName1());
	// launch your tests here
}
