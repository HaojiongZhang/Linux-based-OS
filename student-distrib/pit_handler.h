#ifndef PITHANDLER_H
#define PITHANDLER_H

#ifndef ASM
#include "lib.h"
#include "x86_desc.h"
#include "cp3_syscall.h"
#include "i8259.h"


// PIT I/O Ports  
// source: https://wiki.osdev.org/PIT 

#define PIT_DATA_PORT        0x40   //channel 0
#define PIT_COMMAND_PORT     0x43

#define LOHIBYTE      0x36
#define SQUAREWAVE    0x07
#define COUNTER         11932         

extern void pit_init();
extern void pit_interrupt();

#endif
#endif
