#include "paging.h"

#define READ_WRITE 0x2
#define PRESENT 0x1
#define MB4PAGE 0x80
#define PROGRAM_PCD 0x10
#define USER_SUP 0x4

//create array of 1024 PDE and PTE
PDE page_directory[1024] __attribute__((aligned(4096)));
PTE page_table[1024] __attribute__((aligned(4096)));
PTE vidmap_page_table[1024] __attribute__((aligned(4096)));


/*
Descriptor: this function is used to initialize paging
0-4MB in 4kB pages, 4-8MB as 4MB page
vm at 0xB8000 and kernel at 4-8MB are present
otherwise not present
input: none
return: none*/
void paging_init(){
    int i;

    //initialize all 1024 page directory entries
    for(i = 0; i < 1024; i++){
        page_directory[i].P = 0;
        page_directory[i].RW = 0;
        page_directory[i].US = 0;
        page_directory[i].PWT = 0;
        page_directory[i].PCD = 0;  //one for program code and data pages
        page_directory[i].A = 0; 
        page_directory[i].D = 0;
        page_directory[i].PS = 0;
        page_directory[i].G = 0;    //set to 1 for kernel code
        page_directory[i].AVAIAL = 0;
    }
    //initialize all 1024 page table entries for 0-4MB
    for(i = 0; i < 1024; i++){
        page_table[i].P = 0;
        page_table[i].RW = 0;
        page_table[i].US = 0;
        page_table[i].PWT = 0;
        page_table[i].PCD = 0;  //one for program code and data pages
        page_table[i].A = 0; 
        page_table[i].D = 0;
        page_table[i].PAT = 0;
        page_table[i].G = 0;    //set to 1 for kernel code
        page_table[i].AVAIAL = 0;
    }
    
 
    //kernel at 4-8MB.
    //4MB >> 22 = 0x01, then shift 10 bits for reserved bits
    page_directory[1].Base_address = 1 << 10; // account for Reserved bits
    page_directory[1].PS = 1;   //A large 4 MB Page
    page_directory[1].P = 1;    //Present
    page_directory[1].US = 0;   //Supervisor only for kernel pages
    page_directory[1].RW = 1;
    page_directory[1].PCD = 1;  //one for program code and data pages
    page_directory[1].RW = 1;

    //0-4MB Paging.
    //base address points to the address of page_table shifted
    page_directory[0].Base_address = ((((uint32_t)page_table))>>12);    //accout for 4kb individual pages
    page_directory[0].PS = 0;   //4kB pages
    page_directory[0].P = 1;    //Present
    page_directory[0].US = 0;   //Supervisor only for kernel pages
    page_directory[0].RW = 1;
    page_directory[0].PCD = 0;  //one for program code and data pages
    page_directory[0].RW = 0;

    //video memory
    //base address points to the loctaion in physical mem
    for(i = 0xB8; i < 0xBD; i ++){
        page_table[i].Base_address = i;
        page_table[i].PCD = 0; //PCD cache 0 for video memory page
        page_table[i].US = 1;  // Supervisor only for video memory
        page_table[i].P = 1;
        page_table[i].RW = 1;
        page_table[i].PWT = 0;
    }
    // page_table[0xB9].Base_address = 0xB8;

    page_table[0xBC].Base_address = 0xB8;
    // load  pd and enable paging
    load_directory((uint32_t*)page_directory);
    enable_paging();
}
/*
    Descriptor: this function is used to set the paging for the process with
                pid
    input: pid
    return: none
 */

void
flush_tlb(){
    asm volatile (
 		"movl %%CR3, %%EAX		\n\
 		 movl %%EAX, %%CR3"
 		 : /*no output here*/	  \
 		 : /*no input here*/	  \
 		 : "memory", "cc");		  \
}


void 
set_user_page(uint32_t pid)
{   
    //user memory add by lhy 28/10/2022/21:00
    // 1024 * 1024 used to represent MB
	page_directory[user_space].Base_address = (2 + pid) << 10;
    page_directory[user_space].P = 1;
	page_directory[user_space].PS = 1;
	//page_directory[user_space].A = 1;
    page_directory[user_space].US = 1;
    page_directory[user_space].RW = 1;
	// page_directory[user_space].PS = 1;
    // flushing TLB by rewritting to CR3 (CR7 siuuu!)
	flush_tlb();
}

/*
 *   map_vidmem
 *   DESCRIPTION: allocate and point user program 132MB to video memory, set DPL to 3
 *   INPUTS: 
 *   RETURN VALUE: 
 * 			
 */
void map_vidmem(){
    int i;
    // memory in 132MB
    page_directory[33].Base_address = ((((uint32_t)vidmap_page_table))>>12);
    page_directory[33].P = 1;
    page_directory[33].PS = 0;
    page_directory[33].US = 1;
    page_directory[33].RW = 1;


    i = 0xB8;  //video map location in physical address
    vidmap_page_table[0].Base_address = i;
    vidmap_page_table[0].PCD = 0; //PCD cache 0 for video memory page
    vidmap_page_table[0].US = 1;  // Supervisor only for video memory
    vidmap_page_table[0].P = 1;
    vidmap_page_table[0].RW = 1;
    vidmap_page_table[0].PWT = 0;

    //flush tlb
    flush_tlb();
    return;

}

/*
 *   clear_vidmem
 *   DESCRIPTION: deallocate and turn off memory allocated for user video memory
 *   INPUTS: 
 *   RETURN VALUE: 
 * 			
 */
void clear_vidmem(){
    
    page_directory[33].P = 0; //user mem location @ 132MB
    vidmap_page_table[0].P = 0;  //first entry @ 132MB
    flush_tlb();
    return;

}


/*
 *   set_terminal_page
 *   DESCRIPTION: Helper function for scheduler sets up the video memory for each terminal switch
 *   INPUTS: terminal_num: number of terminal to be switched to
 *   RETURN VALUE: None
 * 			
 */
void set_terminal_page(uint32_t terminal_num)
{
    //store the currently displaying terminal's video memory to 0xB8000 + current terminal's number * 4MB
    memcpy((void*)((0xB8 + dis_terminal) << 12), (void*)0xBC000, 4096);
    //grab the to-be-displayed terminal's video memory and set to display
    memcpy((void*)0xBC000, (void*)((0xB8 + terminal_num) << 12), 4096);
    dis_terminal = terminal_num;
    switch_page();
    return;
}

/*
 *   switch_page
 *   DESCRIPTION: Helper function for set_terminal_page and scheduler that maps the correct video memory  
 *   INPUTS: None
 *   RETURN VALUE: None
 * 			
 */
void
switch_page(){
    //check if displayed terminal on schedule.If so, map video memory to 0xB8
    if(sche_terminal == dis_terminal - 1){
        page_table[0xB8].Base_address = 0xB8;
        vidmap_page_table[0].Base_address = 0xB8;
        }
    //else map to memory storages for the corresponding terminal
    else {
        page_table[0xB8].Base_address = 0xB8 + sche_terminal + 1;
        vidmap_page_table[0].Base_address = 0xB8 + sche_terminal + 1;
    }
    flush_tlb();
}




