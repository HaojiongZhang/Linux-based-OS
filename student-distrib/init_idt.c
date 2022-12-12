/*created by lhy 19:15 13/10/2022
this function is used to handle the initialization of the IDT
the first 20 exception is defined by the hardware of the INTEL CPU
so the initialization wil follow the manual from INTEL.inc*/

/*modified by lhy 16/10/2022 01:11, initialization of IDT*/
#include 	"init_idt.h"
#include 	"wrappers.h"
#include	"cp3_syscall.h"

void init_idt()
{
    int i;
    for(i = 0; i < NUM_VEC; i++){			//for all entries in the IDT table
	idt[i].present     = 0x0;          
	idt[i].dpl         = 0x0;          
	idt[i].reserved0   = 0x0;
	idt[i].size        = 0x1;          
	idt[i].reserved1   = 0x1;   
	idt[i].reserved2   = 0x1;
    idt[i].offset_15_00 = 0x0;
    idt[i].offset_31_16 = 0x0;				//following the INTEL mannual, set the exception to the interrupt gate
	if((i < NUM_EXECPTION) || (i == SYS_CALL_INDEX)){
		idt[i].reserved3   = 0x1;           //for the exceptions, set them to the trap gate
	    }
	else{
	    idt[i].reserved3   = 0x0;           //interrupt into interrupt gate
	    }
	idt[i].reserved4   = 0x0; 
	idt[i].seg_selector = KERNEL_CS;    	//set the mode to kcs, because all interrupt are flaged by kernel
    }
	idt[SYS_CALL_INDEX].reserved3   = 0x0;
    idt[SYS_CALL_INDEX].dpl = 0x03;			//for system call, set it to the highest priviledge
    set_exceptions();
    lidt(idt_desc_ptr);						//load the table into the CPU
}
/*the exception handlers of the first 19 exceptions which are handled by the CPU*/
/*the first exception:devide by 0*/
void exp_0 (){
	cli();
	printf("Divide by zero");
	// while(1){};
	_halt(256);
}

/*the second, debug exception*/
void exp_1 (){
	cli();
    printf("Debug!");
	// 
	_halt(256);
}

/*NMI Interrupt*/
void exp_2 (){
	cli();
    printf("NMI");
	// 
	_halt(256);
}

/*Breakpoint Exception (#BP)*/
void exp_3 (){
	cli();
    printf("Breakpoint!");
	// 
	_halt(256);
}

/*Overflow Exception (#OF)*/
void exp_4 (){
	cli();
	 
    printf("Overflow!");
	// 
	_halt(256);
}

/*BOUND Range Exceeded Exception (#BR)*/
void exp_5 (){
	cli();
    printf("Bound exception!");
	// 
	_halt(256);
}

/*Invalid Opcode Exception (#UD)*/
void exp_6 (){

	 cli();
    printf("Invalid Opcode!");
	// 
	_halt(256);
}

/*Device Not Available Exception (#NM)*/
void exp_7 (){

	cli();
	printf("Device Not Available !");
	// 
	_halt(256);
}

/*Double Fault Exception (#DF)*/
void exp_8 (){

	cli();
	printf("Double Fault!");
	// 
	_halt(256);
}

/*Coprocessor Segment Overrun*/
void exp_9 (){

	cli(); 
	printf("Coprocessor Segment Overrun!");
	// 
	_halt(256);
}

/*Invalid TSS Exception (#TS)*/
void exp_10 (){

	cli();
	printf("Invalid TSS!");
	// 
	_halt(256);
}

/*Segment Not Present (#NP)*/
void exp_11 (){

	 cli();
	printf("Segment not present!");
	// 
	_halt(256);
}

/*Stack Fault Exception (#SS)*/
void exp_12 (){

	cli();
	printf("Stack exception!");
	// 
	_halt(256);
}

/*General Protection Exception (#GP)*/
void exp_13 (){

	cli();
	printf("General Protection!");
	// 
	_halt(256);
}

/*Page-Fault Exception (#PF)*/
void exp_14 (){

	cli();
	printf("Page Fault!");
	// 
	_halt(256);
}

/*x87 FPU Floating-Point Error (#MF)*/
void exp_16 (){

	cli();
	printf("Floating-point!");
	// 
	_halt(256);
}

/*Alignment Check Exception (#AC)*/
void exp_17 (){

	cli();
	printf("Alignment!");
	// 
	_halt(256);
}


/*Machine-Check Exception (#MC)*/
void exp_18 (){

	cli();
	printf("Machine!");
	// 
	_halt(256);
}

/*SIMD Floating-Point Exception (#XF)*/
void exp_19 (){

	cli();
	printf("SIMD!");
	// 
	_halt(256);
}


/*
Descriptor: connect all the first 20 exceptions to the idt entry
also connect the 2 interrupt wrapper entry to the interrupt table 
input: NONE
return: NONE*/
void set_exceptions()
{
	SET_IDT_ENTRY(idt[0],exp_0);
    SET_IDT_ENTRY(idt[1],exp_1);
    SET_IDT_ENTRY(idt[2],exp_2);
    SET_IDT_ENTRY(idt[3],exp_3);
    SET_IDT_ENTRY(idt[4],exp_4);
    SET_IDT_ENTRY(idt[5],exp_5);
    SET_IDT_ENTRY(idt[6],exp_6);
    SET_IDT_ENTRY(idt[7],exp_7);
    SET_IDT_ENTRY(idt[8],exp_8);
    SET_IDT_ENTRY(idt[9],exp_9);
    SET_IDT_ENTRY(idt[10],exp_10);
    SET_IDT_ENTRY(idt[11],exp_11);
    SET_IDT_ENTRY(idt[12],exp_12);
    SET_IDT_ENTRY(idt[13],exp_13);
    SET_IDT_ENTRY(idt[14],exp_14);
    /*ignore exception 15, reserved for intel*/
    SET_IDT_ENTRY(idt[16],exp_16);
    SET_IDT_ENTRY(idt[17],exp_17);
    SET_IDT_ENTRY(idt[18],exp_18);
    SET_IDT_ENTRY(idt[19],exp_19);
    SET_IDT_ENTRY(idt[KB_INDEX], keyboard_wrap);
    SET_IDT_ENTRY(idt[RTC_INDEX], rtc_wrap);
    SET_IDT_ENTRY(idt[PIT_INDEX], pit_wrap);
	SET_IDT_ENTRY(idt[SYS_CALL_INDEX], syscall_linkage);

}



