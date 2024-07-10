
#ifndef _IDT_H
#define _IDT_H

#include "lib.h"
#include "x86_desc.h"
#include "linkage.h"
#include "keyboard.h"

#ifndef ASM

//init the idt table
extern void init_idt();

//sets up exceptions (including 0x80 for system call)
extern void set_exceptions();



//handler functions for exceptions
void divid_error_exc();
void debug_exc();
void nmi_exc();
void breakpoint_exc();
void overflow_exc();
void bound_range_exc();
void invalid_opcode_exc();
void dev_not_avail_exc();
void double_fault_exc();
void coprocessor_exc();
void invalid_tss_exc();
void seg_not_found_exc();
void stack_fault_exc();
void gen_protect_exc();
void page_fault_exc();
void x87_FPU_exc();
void alignment_check_exc();
void machine_check_exc();
void SIMD_FP_exc();

void syscall_wrapper();

//helper function for setting interrupts
void set_interrupts(int idt_num);

//sets up pit interrupts
void set_pit_interrupts();

//sets up keyboard interrupts
void set_keyboard_interrupts();

//sets up rtc interrupts
void set_rtc_interrupts();

void get_cr2();

#endif

#endif 
