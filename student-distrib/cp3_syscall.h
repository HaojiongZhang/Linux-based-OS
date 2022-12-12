#ifndef CP3_SYSCALL_H
#define CP3_SYSCALL_H

#ifndef ASM

#include "lib.h"
#include "x86_desc.h"
#include "init_idt.h"
#include "keyboard.h"
#include "fileSystem.h"
#include "paging.h"
#include "rtc_handler.h"

#define     fail            -1
#define     success         0
#define     max_flength     32
#define     max_process     6
#define     GB              1024 * 1024 * 1024
#define     MB              1024 * 1024
#define     KB              1024
#define     FD_length       8
#define     user_addr       0x08048000

extern int32_t cur_pid;
extern int32_t cur_terminal;
extern int32_t sche_terminal;
extern int32_t dis_terminal;
extern int32_t booting;
extern int32_t sys_execute (const uint8_t * command);
extern int32_t sys_halt (uint8_t status);
extern int32_t _halt (uint32_t status);

extern int32_t read (int32_t fd, void* buf, int32_t nbytes);
extern int32_t write (int32_t fd, void* buf, int32_t nbytes);
extern int32_t open (const uint8_t* filename);
extern int32_t close (int32_t fd);

extern uint32_t get_pcb(uint32_t pid, int mode);

extern uint32_t init_stdIn(int pid);
extern uint32_t init_stdOut(int pid);

extern uint32_t getargs(uint8_t* buf, int32_t nbytes);
extern uint32_t vidmap(uint8_t** screen_start);


extern void syscall_linkage();  //defined system call linkage at cp3_syscal.S
extern void close_stdINOUT();





typedef struct ops {
    int32_t (*read_cb)( int32_t fd, void * buf, int32_t nbytes);
    int32_t (*write_cb)( int32_t fd, void * buf, int32_t nbytes);
    int32_t (*open_cb)(const uint8_t* filename);
    int32_t (*close_cb)(int32_t fd);
} ops_t;

// jump table of operations for dir, terminal, file and rtc.

//struct for file descriptor
typedef struct file_desc{
    ops_t* file_pointer;
    int32_t inode;
    uint32_t file_position;
    uint32_t flags;
} file_desc_t;

//struct for process control block
typedef struct PCB_struct{
    struct PCB_struct * parent_pcb;
    file_desc_t file_descriptor[FD_length];
    uint32_t esp;
	uint32_t ebp;
	uint32_t ss0;
	uint32_t pid;
    uint32_t parent_pid;
    uint8_t* cmd;
    uint8_t* arg;
    uint32_t esp0;
	uint32_t ebp0;
    uint8_t  terminal_id;
} PCB_struct_t;

//struct for terminal
typedef struct terminal {
    PCB_struct_t* cur_active;
    int32_t last_pid;
    uint32_t cursor_x;
    uint32_t cursor_y;
    uint8_t buffer[128];
    uint32_t display_flag;
    uint32_t kb_num;
    uint32_t read_status;
    uint32_t last_esp;
    uint32_t last_ebp;
} terminal_t;

terminal_t terminals[3];

#endif
#endif
