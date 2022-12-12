#ifndef _IDT_H
#define _IDT_H

#ifndef ASM

#include "x86_desc.h"

#include "lib.h"
#include "cp3_syscall.h"
/*the number of the exceptions and the following index of the system call 
and the keyboard and RTC interrupt*/
#define         NUM_EXECPTION       0x20
#define         SYS_CALL_INDEX      0x80
#define         KB_INDEX            0x21
#define         RTC_INDEX           0x28
#define         PIT_INDEX           0x20


void init_idt();
void exp_0 ();
void exp_1 ();
void exp_2 ();
void exp_3 ();
void exp_4 ();
void exp_5 ();
void exp_6 ();
void exp_7 ();
void exp_8 ();
void exp_9 ();
void exp_10 ();
void exp_11 ();
void exp_12 ();
void exp_13 ();
void exp_14 ();
void exp_16 ();
void exp_17 ();
void exp_18 ();
void exp_19 ();

void set_exceptions();

#endif
#endif
