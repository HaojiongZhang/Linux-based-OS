#ifndef SCHEDULER_H
#define SCHEDULER_H

#ifndef ASM
#include "keyboard.h"
#include "lib.h"
#include "paging.h"
#include "x86_desc.h"
#include "cp3_syscall.h"
#include "i8259.h"
extern void scheduler();

#endif
#endif

