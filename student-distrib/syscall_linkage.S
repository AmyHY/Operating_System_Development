#define ASM 1
#include "x86_desc.h"

.global syscall_wrapper
syscall_wrapper:                          
    subl      $1, %eax                      ;\
    cmpl      $0, %eax                      ;\
    jb        invalid                       ;\
    cmpl      $9, %eax                      ;\
    ja        invalid                       ;\
    pushl     %ebp                          ;\
    pushl     %edi                          ;\
    pushl     %esi                          ;\
    pushl     %edx                          ;\
    pushl     %ecx                          ;\
    pushl     %ebx                          ;\
    call      *jmp_table(, %eax, 4)         ;\
    popl      %ebx                          ;\
    popl      %ecx                          ;\
    popl      %edx                          ;\
    popl      %esi                          ;\
    popl      %edi                          ;\
    popl      %ebp                          ;\
    iret                                    ;\
invalid:
    movl      $-1, %eax                     ;\
    iret                                    ;\


jmp_table:
    .long  halt                             ;\
    .long  execute                          ;\
    .long  read                             ;\
    .long  write                            ;\
    .long  open                             ;\
    .long  close                            ;\
    .long  getargs                          ;\
    .long  vidmap                           ;\
    .long  set_handler                      ;\
    .long  sigreturn                        ;
