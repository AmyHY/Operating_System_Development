#include "idt.h"


#define EXC_NUM     20
#define SYS_CALL    0x80
#define PIT         0x20
#define KEYBOARD    0x21
#define RTC         0x28


/* 
 * init_idt
 *   DESCRIPTION: Init the IDT table
 *                Sets up the first 20 exceptions, system call at 0x80, 
 *                RTC and keyboard interrupts
 *   INPUTS: none
 *   OUTPUTS: none
 */

void init_idt() {
    int i;

    //init all entries in the table to some default value
    for (i = 0; i < NUM_VEC; i++) { 
        idt[i].seg_selector = KERNEL_CS;
        idt[i].reserved4 = 0;
        idt[i].reserved3 = 1;                   //change to 0 when setting interrupts
        idt[i].reserved2 = 1;
        idt[i].reserved1 = 1;
        idt[i].size = 1;
        idt[i].reserved0 = 0;
        idt[i].dpl = 0;                         //change to 3 for sys call
        idt[i].present = 0;                     //change to 1 when handler is added
    }

    //sets up first 20 exceptions
    set_exceptions();

    //sets up pit interrupt
    set_pit_interrupts();

    //sets up keyboard interrupt
    set_keyboard_interrupts();

    //sets up rtc interrupt
    set_rtc_interrupts();
}

/* 
 * set_exceptions
 *   DESCRIPTION: sets the first 20 entries for exceptions (except 15 - reserved)
 *                present bit is set to 1
 *                dpl is set to 3 for system call ONLY
 *   INPUTS: none
 *   OUTPUTS: none
 */

void set_exceptions() {
    //set the present bit of exceptions
    int i;
    int reserved_idx = 15;
    for(i = 0; i < EXC_NUM; i++){
        if (i == reserved_idx) {
            continue; 
        }
        idt[i].present = 1;
    }
    //set the present but and DPL bit of the system call
    idt[SYS_CALL].present = 1;
    idt[SYS_CALL].dpl = 3;

    //set the entries of exceptions and system call in IDT. Pass in the wrapped-function address
    SET_IDT_ENTRY(idt[0], &divid_error_exc_wrapper);
    SET_IDT_ENTRY(idt[1], &debug_exc_wrapper);
    SET_IDT_ENTRY(idt[2], &nmi_exc_wrapper);
    SET_IDT_ENTRY(idt[3], &breakpoint_exc_wrapper);
    SET_IDT_ENTRY(idt[4], &overflow_exc_wrapper);
    SET_IDT_ENTRY(idt[5], &bound_range_exc_wrapper);
    SET_IDT_ENTRY(idt[6], &invalid_opcode_exc_wrapper);
    SET_IDT_ENTRY(idt[7], &dev_not_avail_exc_wrapper);
    SET_IDT_ENTRY(idt[8], &double_fault_exc_wrapper);
    SET_IDT_ENTRY(idt[9], &coprocessor_exc_wrapper);
    SET_IDT_ENTRY(idt[10], &invalid_tss_exc_wrapper);
    SET_IDT_ENTRY(idt[11], &seg_not_found_exc_wrapper);
    SET_IDT_ENTRY(idt[12], &stack_fault_exc_wrapper);
    SET_IDT_ENTRY(idt[13], &gen_protect_exc_wrapper);
    SET_IDT_ENTRY(idt[14], &page_fault_exc_wrapper);
    SET_IDT_ENTRY(idt[16], &x87_FPU_exc_wrapper);
    SET_IDT_ENTRY(idt[17], &alignment_check_exc_wrapper);
    SET_IDT_ENTRY(idt[18], &machine_check_exc_wrapper);
    SET_IDT_ENTRY(idt[19], &SIMD_FP_exc_wrapper);

    SET_IDT_ENTRY(idt[SYS_CALL], &syscall_wrapper);   
  
}

/* 
 * set_interrupts
 *   DESCRIPTION: helper function for setting interrupts
 *                present bit is set to 1
 *                reserved3 is set to 0 (interrupt gate)
 *   INPUTS: idt index
 *   OUTPUTS: none
 */

void set_interrupts( int idt_num){
    idt[idt_num].reserved3 = 0;
    idt[idt_num].present = 1;
}
/* 
 * set_pit_interrupts
 *   DESCRIPTION: set the pit idt entry with pit handler
 *   INPUTS: none
 *   OUTPUTS: none
 */
void set_pit_interrupts(){
    set_interrupts(PIT);
    SET_IDT_ENTRY(idt[PIT], &pit_handler_wrapper);
}

/* 
 * set_keyboard_interrupts
 *   DESCRIPTION: set the keyboard idt entry with keyboard handler
 *   INPUTS: none
 *   OUTPUTS: none
 */
void set_keyboard_interrupts(){
    set_interrupts(KEYBOARD);
    SET_IDT_ENTRY(idt[KEYBOARD], &keyboard_handler_wrapper);
}

/* 
 * set_rtc_interrupts
 *   DESCRIPTION: set the rtc idt entry with rtc handler
 *   INPUTS: none
 *   OUTPUTS: none
 */
void set_rtc_interrupts(){
    set_interrupts(RTC);
    SET_IDT_ENTRY(idt[RTC], &rtc_handler_wrapper);
}

//exception function handler
void divid_error_exc(){
    printf("Divide Error Exception\n");
    while(1);
}
void debug_exc(){
    printf("Debug Exception");
    while(1);
}
void nmi_exc(){
    printf("NMI Interrupt");
    while(1);
}
void breakpoint_exc(){
    printf("Breakpoint Exception");
    while(1);
}
void overflow_exc(){
    printf("Overflow Exception");
    while(1);
}
void bound_range_exc(){
    printf("BOUND Range Exceeded Exception");
    while(1);
}
void invalid_opcode_exc(){
    printf("Invalid Opcode Exception");
    while(1);
}
void dev_not_avail_exc(){
    printf("Device Not Available Exception");
    while(1);
}
void double_fault_exc(){
    printf("Double Fault Exception");
    while(1);
}
void coprocessor_exc(){
    printf("Coprocessor Segment Overrun");
    while(1);
}
void invalid_tss_exc(){
    printf("Invalid TSS Exception");
    while(1);
}
void seg_not_found_exc(){
    printf("Segment Not Present");
    while(1);
}
void stack_fault_exc(){
    printf("Stack Fault Exception");
    while(1);
}
void gen_protect_exc(){
    printf("General Protection Exception");
    while(1);
}
void page_fault_exc(){
    printf("Page-Fault Exception");
    get_cr2();
    while(1);
}
void x87_FPU_exc(){
    printf("x87 FPU Floating-Point Error");
    while(1);
}
void alignment_check_exc(){
    printf("Alignment Check Exception");
    while(1);
}
void machine_check_exc(){
    printf("Machine-Check Exception");
    while(1);
}
void SIMD_FP_exc(){
    printf("SIMD Floating-Point Exception");
    while(1);
}

//sys call handler function
void system_call_handler(){
    printf("System Call");
}

void get_cr2(){
    int cr2;
    asm volatile ("movl %%cr2, %0\n" :"=r"(cr2));
    printf("\ncr2 value: %x", cr2);
}




