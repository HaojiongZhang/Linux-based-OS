#include "scheduler.h"


/*
 *   scheduler
 *   DESCRIPTION: scheduler that implements round-robin cpu time allocation for different processes across the three terminals
 *   INPUTS: None
 *   RETURN VALUE: None
 * 			
 */
void
scheduler()
{
    // get esp and ebp registers
    register uint32_t cur_esp asm("esp");
    register uint32_t cur_ebp asm("ebp");
    // start a booting sequence, populating the terminals each with a shell.
    if(booting <= 4){      
        sche_terminal ++;
        booting ++;
        sche_terminal %= 3;
    }
    // open up shell for each terminal
    if(terminals[sche_terminal].cur_active == NULL){
        if(sche_terminal > 0){
            //fill in esp, ebp
            terminals[sche_terminal - 1].last_esp = cur_esp;
            terminals[sche_terminal - 1].last_ebp = cur_ebp;
        }
        switch_page();  //set paging based on whether scheduled terminal
        send_eoi(0);
        sti();
        sys_execute((uint8_t*)"shell"); 
    }
    //booting process finished.
    if(booting == 4) {
        booting = 5;        //just a counter, not a magic number, set to 5 to pretend execute this again.
        sche_terminal = 2;
    }
    terminals[sche_terminal].last_esp = cur_esp;
    terminals[sche_terminal].last_ebp = cur_ebp;
    //keep sche_terminal cycling between 0,1,2
    sche_terminal = (sche_terminal + 1) % 3;
    //find the pid of current active process in scheduled terminal and set up user paging
    set_user_page(terminals[sche_terminal].cur_active -> pid);  
    //fill in tss info
    tss.ss0 = KERNEL_DS;
    tss.esp0 = get_pcb(terminals[sche_terminal].cur_active -> pid, 0);
    switch_page();  //set paging based on whether scheduled terminal
    asm volatile(
        "movl  %0, %%esp   \n\
         movl  %1, %%ebp"
         :
         : "r"(terminals[sche_terminal].last_esp), "r"(terminals[sche_terminal].last_ebp)
         : "memory"
        );
    //detects ctrl+c command, if received terminate the process (halt)
    if (ctrl_c == 1 && dis_terminal - 1 == sche_terminal){
        ctrl_c *= -1;
        send_eoi(0);
        sti();
        sys_halt(0);
    }
    send_eoi(0);

    sti();
}
