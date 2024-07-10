#ifndef LINKAGE_H
#define LINKAGE_H

#ifndef ASM

//exception wrapper
void divid_error_exc_wrapper();
void debug_exc_wrapper();
void nmi_exc_wrapper();
void breakpoint_exc_wrapper();
void overflow_exc_wrapper();
void bound_range_exc_wrapper();
void invalid_opcode_exc_wrapper();
void dev_not_avail_exc_wrapper();
void double_fault_exc_wrapper();
void coprocessor_exc_wrapper();
void invalid_tss_exc_wrapper();
void seg_not_found_exc_wrapper();
void stack_fault_exc_wrapper();
void gen_protect_exc_wrapper();
void page_fault_exc_wrapper();
void x87_FPU_exc_wrapper();
void alignment_check_exc_wrapper();
void machine_check_exc_wrapper();
void SIMD_FP_exc_wrapper();


//pit wrapper
void pit_handler_wrapper();

//keyboard wrapper
void keyboard_handler_wrapper();

//rtc wrapper
void rtc_handler_wrapper();

#endif

#endif
