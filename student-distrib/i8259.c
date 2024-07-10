/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */

#define READ_IRR            0x0A
#define READ_ISR            0x0B
#define IR2                 0x02

/* Initialize the 8259 PIC */
void i8259_init(void) {
    
    //mask all interrupts (0xFF = 1111 1111 -> all interrupts are masked)
    outb(0xFF, MASTER_DATA);  
    outb(0xFF, SLAVE_DATA);

    //init PIC
    outb(ICW1, MASTER_8259_PORT); // starts init master PIC
    outb(ICW1, SLAVE_8259_PORT); // starts init slave PIC

    //map interrupt vectors
    outb(ICW2_MASTER, MASTER_DATA);
    outb(ICW2_SLAVE, SLAVE_DATA);

    //master/slave relationship
    outb(ICW3_MASTER, MASTER_DATA);
    outb(ICW3_SLAVE, SLAVE_DATA);

    //setting modes
    outb(ICW4, MASTER_DATA);
    outb(ICW4, SLAVE_DATA);

    master_mask = 0xFB; // FB = 1111 1011 -> mask all interrupts except IR2 for slave 
    slave_mask = 0xFF; //FF = 1111 1111 -> mask all interrupts on slave

    //maks all interrupts except IR2 for slave PIC
    outb(master_mask, MASTER_DATA); 
    outb(slave_mask, SLAVE_DATA);

}

/* Enable (unmask) the specified IRQ */
void enable_irq(uint32_t irq_num) {
    
    //check if master PIC (IRQ 0 - 7)
    if(irq_num < 8){
        master_mask = (master_mask & ~(1 << irq_num));
        outb(master_mask, MASTER_DATA);
    }
    //slave PIC (IRQ 8 - 15 )
    else{
        irq_num = irq_num - 8;
        slave_mask = (slave_mask & ~(1 << irq_num));
        outb(slave_mask, SLAVE_DATA);
    }
}

/* Disable (mask) the specified IRQ */
void disable_irq(uint32_t irq_num) {
    
    //check if master PIC (IRQ 0 - 7)
    if(irq_num < 8){
        master_mask = (master_mask | (1 << irq_num));
        outb(master_mask, MASTER_DATA);
    }
    //slave PIC (IRQ 8 - 15 )
    else{
        irq_num = irq_num - 8;
        slave_mask = (slave_mask | (1 << irq_num));
        outb(slave_mask, SLAVE_DATA);
    }
}

/* Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num) {
    
    //check if master PIC (IRQ 0 - 7)
    if(irq_num < 8){
        outb((EOI | irq_num), MASTER_8259_PORT);

    }
    //slave PIC (IRQ 8 - 15 ) 
    //send eoi on irq2 for master and on irq_num offset on slave
    else{
        outb((EOI | IR2), MASTER_8259_PORT);
        outb((EOI | (irq_num-8)), SLAVE_8259_PORT);
    }
}

