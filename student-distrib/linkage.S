#define ASM     1

#include "linkage.h"

//linkage macro which allows for easier linkage for exceptions, interrupts,
//and handlers

#define INTR_LINK(name, func)       \
    .global name                    ;\
    name:                           ;\
        pushal                      ;\
        pushfl                      ;\
        call func                   ;\
        popfl                       ;\
        popal                       ;\
        iret                        ;\

//exceptions wrappers
INTR_LINK(divid_error_exc_wrapper, divid_error_exc);
INTR_LINK(debug_exc_wrapper,debug_exc);
INTR_LINK(nmi_exc_wrapper, nmi_exc);
INTR_LINK(breakpoint_exc_wrapper, breakpoint_exc);
INTR_LINK(overflow_exc_wrapper, overflow_exc);
INTR_LINK(bound_range_exc_wrapper, bound_range_exc);
INTR_LINK(invalid_opcode_exc_wrapper, invalid_opcode_exc);
INTR_LINK(dev_not_avail_exc_wrapper, dev_not_avail_exc);
INTR_LINK(double_fault_exc_wrapper, double_fault_exc);
INTR_LINK(coprocessor_exc_wrapper, coprocessor_exc);
INTR_LINK(invalid_tss_exc_wrapper, invalid_tss_exc);
INTR_LINK(seg_not_found_exc_wrapper, seg_not_found_exc);
INTR_LINK(stack_fault_exc_wrapper, stack_fault_exc);
INTR_LINK(gen_protect_exc_wrapper, gen_protect_exc);
INTR_LINK(page_fault_exc_wrapper, page_fault_exc);
INTR_LINK(x87_FPU_exc_wrapper, x87_FPU_exc);
INTR_LINK(alignment_check_exc_wrapper, alignment_check_exc);
INTR_LINK(machine_check_exc_wrapper, machine_check_exc);
INTR_LINK(SIMD_FP_exc_wrapper, SIMD_FP_exc);

//sys call wrapper
INTR_LINK(system_call_handler_wrapper, system_call_handler);

//pit wrapper
INTR_LINK(pit_handler_wrapper, pit_handler);

//keyboard wrapper
INTR_LINK(keyboard_handler_wrapper, keyboard_handler);

//rtc wrapper
INTR_LINK(rtc_handler_wrapper, rtc_handler);
