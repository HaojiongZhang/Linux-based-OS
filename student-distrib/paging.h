#ifndef PAGING_H
#define PAGING_H


#include "x86_desc.h"
#include "cp3_syscall.h"
#ifndef ASM
// #include "cp3_syscall.h"

extern void paging_init();

#define     user_space      32   //     128mb >> 22
#define     video_memory       0xB8000
/*
Descriptor: this function loads page directory to CR3
input: pointer to address of page directory
return: none*/
extern void load_directory(uint32_t * directory);

extern void set_user_page(uint32_t pid);
/*
Descriptor: this function enables paging.
input: none
return: none*/
extern void enable_paging();

extern void map_vidmem();

extern void clear_vidmem();

extern void set_terminal_page(uint32_t terminal_num);

extern void switch_page();

extern void fluch_tlb();

#endif
#endif
