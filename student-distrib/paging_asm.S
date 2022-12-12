#define ASM 1
#include "paging.h"
.text 
.globl load_directory
.globl enable_paging
load_directory:
    pushl %ebp			
  	movl %esp, %ebp
    pushl %ebx 
  	pushl %esi 
  	pushl %edi

# Refrence from: https://wiki.osdev.org/Paging
    movl 8(%ebp), %eax   
    movl %eax, %cr3
    
    movl %cr4, %eax
    orl  $0x00000010, %eax
    movl %eax, %cr4
   
   

    popl %edi
    popl %esi
    popl %ebx
    leave
    ret

enable_paging:
    pushl %ebp			
  	movl %esp, %ebp
    pushl %ebx 
  	pushl %esi 
  	pushl %edi

# Refrence from: https://wiki.osdev.org/Paging
    movl %cr0, %eax
    orl $0x80000001, %eax
    movl %eax, %cr0
 

    popl %edi
    popl %esi
    popl %ebx
    leave
    ret


    

