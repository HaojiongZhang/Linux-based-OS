#include "cp3_syscall.h"
#include "x86_desc.h"

/*system call execute, created by lhy*/

/*updated 29/10/2022/13:45*/
// PCB_struct_t* curPCB = (8 * MB - cur_esp) / (8 * KB);

static int pid[max_process];
int32_t cur_pid = -1;                 // used to record the current process id originally -1


//struct for 4 funtions in each jump table entries.
static ops_t directory_table = {dir_read, dir_write, dir_open, dir_close};
static ops_t terminal_table = {terminal_read, terminal_write, terminal_open, terminal_close};
static ops_t file_table = {file_read, file_write, file_open, file_close};
static ops_t rtc_table = {rtc_read, rtc_write, rtc_open, rtc_close};

int32_t dis_terminal = 1;
int32_t cur_terminal = 0;
int32_t sche_terminal = -1;
int32_t booting = 0;
/*
 *   close_stdINOUT
 *   DESCRIPTION: close stdIO via this special function during halt, prevents stdIO being
 *   closed using close().
 *   INPUTS: None
 *   RETURN VALUE: None
 * 			
 */
void close_stdINOUT(){
    // register uint32_t cur_esp asm("esp");
    PCB_struct_t * curPCB;
    curPCB = terminals[sche_terminal].cur_active;   //get current PCB from terminal
    if(curPCB->file_descriptor[0].flags == 0 || curPCB->file_descriptor[1].flags == 0){ //check if descriptor in use  
        return;
    }
    //Do clean up fd[0] and fd[1] and clean spaces.
    curPCB->file_descriptor[0].file_pointer->close_cb(0);

    curPCB->file_descriptor[0].file_pointer = NULL;
    curPCB->file_descriptor[0].flags = 0;
    curPCB->file_descriptor[0].inode = 0;
    curPCB->file_descriptor[0].file_position = 0;

    curPCB->file_descriptor[1].file_pointer->close_cb(1);

    curPCB->file_descriptor[1].file_pointer = NULL;
    curPCB->file_descriptor[1].flags = 0;
    curPCB->file_descriptor[1].inode = 0;
    curPCB->file_descriptor[1].file_position = 0;
    return;
}


/*
 *   sys_execute
 *   DESCRIPTION: execute process based on command if there is still free space for process
 *   and read_entry_by_name identifies the file and it is executable. Also keeps track of parent process id.
 *   INPUTS: command: the command to be executed
 *   RETURN VALUE: Success: 0, else -1
 * 			
 */
int32_t 
sys_execute (const uint8_t * command)
{
    cli();   // temporary var
    int i,j,k,l, find_pid, eip_addr;
    // esp ebp registers
    register uint32_t cur_esp asm("esp");
    register uint32_t cur_ebp asm("ebp");
    uint8_t EIP[4];
    // process control block
    PCB_struct_t * pcb;
    dentry_t dentry;
    // uint8_t fname[max_flength];
    uint8_t read_buf[max_flength];

    uint8_t parsed_cmd[1025];
    uint8_t parsed_arg[1025];


    //initialize parsing buffers
    for(i = 0; i<1025; i++){
        parsed_cmd[i] = '\0';
        parsed_arg[i] = '\0';
    }
    i = 0;
    while(command[i] == ' ' || command[i] == '\t'){
        i++;    //jump space at head
    }
    j = 0;
    while(command[i+j] != ' ' && command[i+j] != '\n' && command[i+j] != '\t' && command[i+j] != '\0'){
        j++;    //find command end
    }
    for(k = 0; k <(j); k++){
        parsed_cmd[k] = command[i+k];   //store command
    }
    i = i+j;//6
    if(command[i] == ' ' || command[i] == '\t'){ 
        l = i;
        while(command[i] == ' ' || command[i] == '\t'){
            i++;    // jump space before args   
        }  
        l = i;
        j = 0; //j is counter;
        while(command[i] != ' ' && command[i] != '\n' && command[i] != '\t' && command[i] != '\0'){
            i++;
            j++;
        }   
        for(k = 0; k <(j); k++){
            parsed_arg[k] = command[l+k];   //store args
        }
    }
    //pass command
    command = parsed_cmd;

    // read dentry and data
    if(read_dentry_by_name (command, &dentry) == fail) return fail;             //find the command in the file system
    if(read_data (dentry.inode_num, 0, read_buf, 32) == fail) return fail;      //read the daa of the file
    //for(i = 0; i < 4; i ++) EIP += (int32_t)read_buf[23 + i] << ((3 - i) * 8);
    // check for program headings 
    read_data (dentry.inode_num, 24, EIP, 4);           //read the start address pointer of the program
    eip_addr = *((int32_t*)EIP);
    if(read_buf[0] != 0x7f || read_buf[1] != 0x45 || read_buf[2] != 0x4c || read_buf[3] != 0x46) return fail;       //check if the head of file is ?ELF
    // find free pid
    for(i = 0; i < max_process; i++){
        if(pid[i] == 0){
            pid[i] = 1;
            find_pid = 1;           //find a place for the process
            break;
        }
    }
    // no free pid available
    if(find_pid == 0){
        printf("Reached maximum number of processes \n");
        return fail;
    }
    set_user_page(i);
    read_data (dentry.inode_num, 0, (uint8_t *) user_addr, (128 + 4) * MB - user_addr);
    // set up paging for the pid process
    
    pcb = (PCB_struct_t *)(8 * MB - (i + 1) * 8 * KB); //
    // switch context 
    pcb->pid = i;
    pcb->terminal_id = dis_terminal - 1;
    // assign parent pid
    if(terminals[sche_terminal].cur_active == NULL) pcb->parent_pid = -1;
    else pcb->parent_pid = terminals[sche_terminal].cur_active -> pid; 
    // if(booting <= 3) terminals[sche_terminal].last_pid = i;  //store current pid into my child's pcb
    // else terminals[(sche_terminal + 1) % 3].last_pid = i;
    if (cur_pid != -1) pcb->parent_pcb = (PCB_struct_t *)(8 * MB - (pcb->parent_pid + 1) * 8 * KB);     //if not is the first shell, reboot
    terminals[sche_terminal].cur_active = pcb;//set PCB of scheduled terminal
    pcb->esp = cur_esp;
    pcb->ebp = cur_ebp;             //store current ebp&ep into my child's pcb
    pcb->arg = parsed_arg;
    pcb->cmd = parsed_cmd;
    

    // update the current id 
    cur_pid = i;
    init_stdIn(i);
    init_stdOut(i);                 // initialize the standard in/out
    tss.ss0 = KERNEL_DS;            
    tss.esp0 = get_pcb(i, 0);       //load the destination stack pointer into esp0
    sti();
    asm volatile(
        "pushl  %0   \n\
         pushl  %1   \n\
         pushfl      \n\
         pushl  %2   \n\
         pushl  %3   \n\
         iret        \n"             //iret calling convention from osdev.org
         :
         : "r"(USER_DS), "r"(132 * MB - 4), "r"(USER_CS), "r"(eip_addr) //132 MB - 4 is the top of the user level stack, which is used as an esp pointer.
         : "memory"
    );
    return SUCCESS;
} 

/*
 *   _halt
 *   DESCRIPTION: halt process. Close all files opened by the process and close stdIn, stdOut.
 *   check if the curr process is shell, if halt shell, restart a new shell process. Free the pid 
 *   and restore parent paging, then return.
 *   INPUTS: status: return value needed by kernel
 *   RETURN VALUE: Success: 0, else -1
 * 			
 */
int32_t 
_halt (uint32_t status)
{
// store the current esp and ebp
    int i;
    uint32_t esp, ebp;

    PCB_struct_t * curPCB;
    curPCB = terminals[sche_terminal].cur_active; //get current PCB from terminal
    /*close any relevant FDs*/
    for (i = 2; i < FD_length; i++){
        close(i);
    }
    close_stdINOUT();   //close stdIn and stdOut
    /*restore parent data*/
    int32_t parent_id = terminals[sche_terminal].cur_active->parent_pid;
    terminals[sche_terminal].last_pid = parent_id;
    // current process is at shell, halt causing restart the shell
    cur_pid = terminals[sche_terminal].cur_active -> pid;
    pid[cur_pid] = 0;  //clear cur_pid
    // clear_vidmem();
    if (parent_id == -1){
        terminals[sche_terminal].cur_active = NULL;
        sys_execute((uint8_t*)("shell"));
    }
    else{
        /*restore parent paging*/
        tss.ss0 = KERNEL_DS;
        tss.esp0 = get_pcb(parent_id, 0);
        set_user_page(parent_id);
        esp = terminals[sche_terminal].cur_active->esp;
        ebp = terminals[sche_terminal].cur_active->ebp;
        terminals[sche_terminal].cur_active = terminals[sche_terminal].cur_active -> parent_pcb;
        /*Jump to execute return*/
        asm volatile ("         \n\
            movl  %%ebx, %%esp  \n\
            movl  %%ecx, %%ebp  \n\
            movl  %%edx, %%eax  \n\
            leave               \n\
            ret                 \n"
            :
            : "b"(esp), "c"(ebp), "d"(status)
            : "eax", "ebp", "esp"
        );
    }
    // eliminate warnings
    return SUCCESS;

} 

/*
 *   sys_halt
 *   DESCRIPTION: wrapper of _halt to match uint8_t parameter requirement. Calls _halt 
 *   with typecasted given input of status 
 *   INPUTS: status: input to _halt
 *   RETURN VALUE: Success: 0, else -1
 * 			
 */
int32_t 
sys_halt (uint8_t status)
{
    if((_halt((uint32_t)status)) == SUCCESS){
        return SUCCESS;
    }
    
    return -1;
}



/*
 *   get_pcb
 *   DESCRIPTION: Helper function of sys_execute. given process id and mode, get pid or 
 *   location of PCB based on mode 
 *   INPUTS: pid: process id; mode: 1 for pid, mode 0 for pcb location
 *   RETURN VALUE: Success: pid / location of PCB.
 * 			
 */
uint32_t 
get_pcb(uint32_t pid, int mode)
{   
    register uint32_t cur_esp asm("esp");
    //if(pid == 0) return 8 * MB;
    if(mode) return (8 * MB - cur_esp) / (8 * KB); //pid
    return (8 * MB - (pid) * 8 * KB - 4);
}


/*
 *   read
 *   DESCRIPTION: Call read funtion of the corresponding table, with parameter
 *   usage specific to each case's implementation.
 *   INPUTS: fd index, buf, nbytes
 *   RETURN VALUE: Success: retval of read function, usually num bytes read. else return -1
 * 			
 */
int32_t read (int32_t fd, void* buf, int32_t nbytes){
    PCB_struct_t* curPCB;
    register uint32_t cur_esp asm("esp");
    int pid, ret;
    sti();
    pid = (8 * MB - cur_esp) / (8 * KB);
    curPCB = terminals[sche_terminal].cur_active; //get current PCB from terminal
    if (fd < 0 || fd>7){  //invalid descriptor
        return -1;
    }

    if(curPCB->file_descriptor[fd].flags == 0){ //check if descriptor in use  
        return -1;
    }

    if (buf == NULL){
        return -1;
    }

    if(nbytes < 0){
        return -1;
    }

    //Do write function upon the fd entry, usually return # of bytes read
    ret = curPCB->file_descriptor[fd].file_pointer->read_cb(fd,buf,nbytes);
    return ret;
}
/*
 *   write
 *   DESCRIPTION: Call write funtion of the corresponding table, with parameter
 *   usage specific to each case's implementation.
 *   INPUTS: fd index, buf, nbytes
 *   RETURN VALUE: Success: 0. else return -1
 * 			
 */
int32_t write (int32_t fd, void* buf, int32_t nbytes){
    PCB_struct_t * curPCB;
    register uint32_t cur_esp asm("esp");
    int pid;
    pid = (8 * MB - cur_esp) / (8 * KB);
    curPCB = terminals[sche_terminal].cur_active; //get current PCB from terminal

    if (fd < 0 || fd>7){  //check invalid descriptor index
        return -1;
    }

    if(curPCB->file_descriptor[fd].flags == 0){ //check if descriptor in use  
        return -1;
    }

    //sanity check 
    if (buf == NULL){
        return -1;
    }
    if(nbytes < 0){
        return -1;
    }

    //Do write function upon the fd entry
    return curPCB->file_descriptor[fd].file_pointer->write_cb(fd,buf,nbytes);
    return SUCCESS;
}

/*
 *   open
 *   DESCRIPTION: Open the file using the given filename, and if successfully found, open and
 *   insert corresponding enrty to fd array.
 *   INPUTS: fd index 
 *   RETURN VALUE: Success: fd index of the file opened. else return -1
 * 			
 */
int32_t open (const uint8_t* filename){
    int32_t fileType, fd, tmp;
    register uint32_t cur_esp asm("esp");
    dentry_t curFile;
    PCB_struct_t * curPCB;
    int pid;
    pid = (8 * MB - cur_esp) / (8 * KB);
    int i, length;
    int ret;

    //check valid input
    curPCB = terminals[sche_terminal].cur_active; //get current PCB from terminal
    length =  strlen((int8_t*)filename);
    if (filename == NULL || length < 1){ 
        return -1;
    }

    //read the file by name, check ret val.
    tmp = read_dentry_by_name(filename, &curFile);
    if(tmp == -1){
        return -1;
    }

    //use filetype to locate correct jump table
    fileType = curFile.fileType;        

    //find next available entry in file descriptor array
    for(i = 0; i < 8; i++){
        if(curPCB->file_descriptor[i].flags == 0){
            fd = i;
            break;
        }
        if(i == 7){   //unable to find open entry in file descriptor array
            return -1;  
        }
    }


    //fileType:
    // 0: RTC
    // 1: directory
    // 2: regular file
    // set file_pointer to the corresponding table
    switch (fileType)
    {
    case 0:   //RTC

        curPCB->file_descriptor[fd].flags = 1;
        curPCB->file_descriptor[fd].file_pointer = &rtc_table;
        curPCB->file_descriptor[fd].inode = 0;
        curPCB->file_descriptor[fd].file_position = 0;

        break;
    case 1: //Directory
        //store the current file dentry and offset positon at curFile. 
        cpyFileDescriptor((uint32_t*)&(curPCB->file_descriptor[fd].file_position), fd, (uint8_t*)filename);

        curPCB->file_descriptor[fd].flags = 1;
        curPCB->file_descriptor[fd].file_pointer = &directory_table;
        curPCB->file_descriptor[fd].inode = 0;
        curPCB->file_descriptor[fd].file_position = 0;
        break;
    case 2://File
        //store the current file dentry and offset positon at curFile. 
        cpyFileDescriptor((uint32_t*)&(curPCB->file_descriptor[fd].file_position), fd, (uint8_t*)filename);

        curPCB->file_descriptor[fd].flags = 1;
        curPCB->file_descriptor[fd].file_pointer = &file_table;
        curPCB->file_descriptor[fd].inode = curFile.inode_num;
        curPCB->file_descriptor[fd].file_position = 0;
        break;
    default: //wrong file type. return.
        return -1;
        break;
    }
    
    //Do open function upon the fd entry
    ret = curPCB->file_descriptor[fd].file_pointer->open_cb(filename);
    if(ret == -1){      //unable to open correct file
        return -1;
    }
    return fd;
}


/*
 *   close
 *   DESCRIPTION: Close the file indexed by the file discriptor array index fd, and
 *   remove corresponding enrty from fd array.
 *   INPUTS: fd index 
 *   RETURN VALUE: 0 if Successful, -1 if not.
 * 			
 */
int32_t close (int32_t fd){
    PCB_struct_t * curPCB;
    register uint32_t cur_esp asm("esp");
    int pid;
    pid = (8 * MB - cur_esp) / (8 * KB);
    curPCB = terminals[sche_terminal].cur_active; //get current PCB from terminal



    if (fd < 2 || fd>7){  //check invalid descriptor index. Don't mess with stdIO unless halting.
        return -1;
    }

    if(curPCB->file_descriptor[fd].flags == 0){ //check if file is open and in use  
        return -1;
    }

    //Do close function upon the fd entry
    curPCB->file_descriptor[fd].file_pointer->close_cb(fd);
    //Clear space of file descriptor. Set ptr to NULL and flag =0.
    curPCB->file_descriptor[fd].file_pointer = NULL;
    curPCB->file_descriptor[fd].flags = 0;
    curPCB->file_descriptor[fd].inode = 0;
    curPCB->file_descriptor[fd].file_position = 0;

    return 0;
}


/*
 *   init_stdIn
 *   DESCRIPTION: Helper func of sys_execute. Given a process id, find the PCB associated and
 *   initialize fd[0] to terminal table to enable stdIn.
 *   INPUTS: process id.
 *   RETURN VALUE: 1 if Successful.
 * 			
 */
uint32_t init_stdIn(int pid){
    //int pid;
    //register uint32_t cur_esp asm("esp");
    PCB_struct_t * curPCB;
    curPCB = terminals[sche_terminal].cur_active; //get current PCB from terminal
    curPCB = (PCB_struct_t *)(8 * MB - (pid + 1) * 8 * KB);         //find the position of the current pcb referenced by the pid
    curPCB->file_descriptor[0].flags = 1;
    curPCB->file_descriptor[0].file_pointer = &terminal_table;      //using termminal table, zero filesys parameters
    curPCB->file_descriptor[0].inode = 0;
    curPCB->file_descriptor[0].file_position = 0;
    curPCB->file_descriptor[0].file_pointer->open_cb(0);
    return 1;
}


/*
 *   init_stdOut
 *   DESCRIPTION: Helper func of sys_execute. Given a process id, find the PCB associated and
 *   initialize fd[1] to terminal table to enable stdOut.
 *   INPUTS: process id.
 *   RETURN VALUE: 1 if Successful.
 * 			
 */
uint32_t init_stdOut(int pid){
    //int pid;
    //register uint32_t cur_esp asm("esp");
    PCB_struct_t * curPCB;
    curPCB = terminals[sche_terminal].cur_active;//get current PCB from terminal
    curPCB = (PCB_struct_t *)(8 * MB - (pid + 1) * 8 * KB);         //find the position of the current pcb referenced by the pid
    curPCB->file_descriptor[1].flags = 1;
    curPCB->file_descriptor[1].file_pointer = &terminal_table;      //using termminal table, zero filesys parameters
    curPCB->file_descriptor[1].inode = 0;
    curPCB->file_descriptor[1].file_position = 0;
    curPCB->file_descriptor[1].file_pointer->open_cb(0);
    return 1;

}


/*
 *   getargs
 *   DESCRIPTION: given a buffer, copy the arguments of current command into the buffer
 *   INPUTS:  buf: location to copy arg to
 *            nbytes: number of bytes to copy
 *   RETURN VALUE: Success: 0
 * 			
 */
uint32_t getargs(uint8_t* buf, int32_t nbytes){
    uint8_t* tmp;
    // pid = (8 * MB - cur_esp) / (8 * KB);
    // terminals[sche_terminal].cur_active  = (PCB_struct_t *)(8 * MB - (pid + 1) * 8 * KB);    //get pid and then cur PCB from esp  
    tmp = terminals[sche_terminal].cur_active -> arg;
    memcpy(buf, tmp, nbytes);
    return 0;

}



/*
 *   vidmap
 *   DESCRIPTION: allocate and point pages to video memory, set DPL to 3
 *   INPUTS: screen_start: double pointer to start of video memory
 *   RETURN VALUE: Success: 1, else -1
 * 			
 */

uint32_t vidmap(uint8_t** screen_start){
    if(screen_start == NULL || screen_start < (uint8_t**)(128 * MB) || screen_start > (uint8_t**)(132 * MB)){ //check for null and oob
        return -1;
    }
    //need to check bound
    map_vidmem();
    *screen_start = (uint8_t *)(132*MB);
    return 1;
}


