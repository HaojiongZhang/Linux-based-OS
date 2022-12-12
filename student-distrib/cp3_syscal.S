#define ASM 1
#include "cp3_syscall.h"

.globl syscall_linkage

syscall_linkage:
    pushl %ebp
    pushl %edi
    pushl %esi    
    decl %eax
    cmpl $8, %eax
    jg invalid
    cmpl $0, %eax
    jl invalid
    movl syscall_jmp_table(, %eax, 4), %eax
    pushl %edx
    pushl %ecx
    pushl %ebx
    call *%eax
    popl %ebx
    popl %ecx
    popl %edx
    jmp end
invalid:
    movl $-1, %eax
end:
    popl %esi
    popl %edi
    popl %ebp
    IRET


syscall_jmp_table:
    .long sys_halt
    .long sys_execute
    .long read
    .long write
    .long open
    .long close
    .long getargs
    .long vidmap
    # .long syscall_set_handler
    # .long syscall_sigreturn
