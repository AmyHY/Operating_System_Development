#include "rtc.h"
#include "lib.h"

#define RTC_IRQ       0x08
#define RTC_INDEX     0x70
#define CMOS          0x71
#define register_A    0x8A 
#define register_B    0x8B
#define register_C    0x0C //status register
#define RATE_1024_HZ  0x06 // when rate = 6, frequency = 32768/(2^(rate - 1)) = 1024
#define FREQ_MAX      1024
//int rtc_interrupt_occurred;
//int max_rtc_count_before_show; // 2 = min freq; each round is 1024 Hz
//int rtc_count; //count the rtc int in current round


/* 
 * init_rtc
 *   DESCRIPTION: turn up rtc, enable (unmask) IRQ for rtc
 *   and reads the status register
 *   INPUTS: none
 *   OUTPUTS: none
 */
void init_rtc(){ //follows instruction on osdev
    //turn on IRQ8
    unsigned int flags;
    cli_and_save(flags);  
    outb(register_B, RTC_INDEX);
    char prev = inb(CMOS); //CMOS_IO_PORT reads the curr value of register B
    outb(register_B, RTC_INDEX);
    outb(prev | 0x40, CMOS); // 0x40 = 0100 0000, so this turns on bit 6 of register B
    restore_flags(flags);
    enable_irq(RTC_IRQ);
    read_register_C();
}




/* 
 * rtc_handler
 *   DESCRIPTION: messes up the screen to show that rtc int is received
 *   INPUTS: none
 *   OUTPUTS: none
 */
void rtc_handler(){
    //if rtc has been opened
    //increment counter (file pos) for all process that has rtc opened
    send_eoi(RTC_IRQ);
    uint32_t pid;
    for(pid = 0; pid < MAX_PID; pid++){
        //skip if process is not set
        if(pid_status[pid] == 1){
            pcb_t* pcb = get_pcb(pid);
            //check if current process has opened rtc (rtc_fd_idx is -1 if not opened)
            //note rtc_fd_idx is set in the open syscall
            uint32_t rtc_fd = pcb->rtc_fd_idx;
            if(rtc_fd != -1){
                pcb->fd_array[rtc_fd].file_pos = pcb->fd_array[rtc_fd].file_pos + 1;  //increment counter

                //check counter has reached max count
                if(pcb->fd_array[rtc_fd].file_pos == pcb->max_rtc_count){
                    pcb->rtc_interrupt = 1;                             //signal interrupt 
                    pcb->fd_array[rtc_fd].file_pos = 0;        //reset counter
                }
                else{
                    pcb->rtc_interrupt = 0;
                }
            }
        }

    }

  
    
    read_register_C();
    
}




/* 
 * read_register_C
 *   DESCRIPTION: reads the status register so
 *   that rtc proceeds to give interrupts
 *   INPUTS: none
 *   OUTPUTS: none
 */
void read_register_C() {
    outb(register_C, RTC_INDEX);
    inb(CMOS);
}




/* 
 * rtc_read
 *   DESCRIPTION: block a flag until the next rtc inerrupt
 *   INPUTS: fd - file descriptor
 *           buf - buffer
 *           nbytes - should be 4 
 *   OUTPUTS: 0 on success, -1 on failure
 */
int rtc_read(int32_t fd, void* buf, int32_t nbytes){
    // synchronize with interrupt handler 
    //set a flag and wait until the interrupt handler clears it.

    //sanity check
    if (buf == NULL || (fd < 0 || fd >= 8)) { //8 is size of file array
        //4: the system call should always accept only a 4-byte integer specifying the interrupt rate in Hz
        return -1;
    }

     pcb_t* pcb = get_pcb(schedule[active_term_idx]);
     
     //if the current process has rtc open, block until interrupt has occured
     //if(pcb->rtc_fd_idx != -1){

       while(!(pcb->rtc_interrupt)){};
        pcb->rtc_interrupt = 0;
    
     //}
    
    return 0;
     
}  






/* 
 * rtc_write
 *   DESCRIPTION: Change the rtc frequency into value in buf
 *   INPUTS: fd - file descriptor
 *           buf - buffer that stores the desired frequency
 *           nbytes - should be 4
 *   OUTPUTS: number of bytes written (should be 4)
 */
int rtc_write(int32_t fd, const void* buf, int32_t nbytes){ 
    //be able to change the frequency.
    //Function arg is in Hz. Returns the number of bytes written.

    int f;
    
    if (buf == NULL || nbytes != 4 || (fd < 0 || fd >= 8)) { //8 is size of file array
        //4: the system call should always accept only a 4-byte integer specifying the interrupt rate in Hz
        return -1;
    }

    f = *(int*)(buf);
    //check for valid frequency
    if (f < 2 || f > FREQ_MAX) { // 2 = min freq.
        return -1;
    }

    pcb_t* pcb = get_pcb(schedule[active_term_idx]);
    pcb->rtc_fd_idx = fd; 
    pcb->max_rtc_count = FREQ_MAX / f; //update the max count of rtc interrupt per 1024 ticks

    return sizeof(int32_t);
}





/* 
 * rtc_open
 *   DESCRIPTION: initializes RTC frequency to 2Hz.
 *   INPUTS: filename - unused, but needs null check
 *   OUTPUTS: 0 on success, -1 on failure
 */
int rtc_open(const uint8_t* filename){ //initializes RTC frequency to 2Hz.
    if (filename == NULL ) {
        return -1;
    }
    pcb_t* pcb = get_pcb(schedule[active_term_idx]);
    pcb->max_rtc_count = FREQ_MAX / FREQ_MAX; // default: freq 1024hz -> count before = 1
   
    //set frequency		
    rtc_set_rate(RATE_1024_HZ);
    return 0;
}




/* 
 * rtc_close
 *   DESCRIPTION: does nothing unless we virtualize RTC
 *   INPUTS: fd - file descriptor. Unused, but needs null check
 *   OUTPUTS: 0 on success, -1 on failure
 */
int rtc_close(int32_t fd){//does nothing unless we virtualize RTC
    if (fd < 0 || fd >= 8 ) { //fd goes from 0 to 8. because 8 is the size of file array
        return -1;
    }
    pcb_t* pcb = get_pcb(schedule[active_term_idx]);
    pcb->max_rtc_count = 0;
    pcb->rtc_interrupt = 0;
    pcb->rtc_fd_idx = -1;
    return 0;
}




/* 
 * rtc_set_rate
 *   DESCRIPTION: helper function to set frequency, given a rate
 *   INPUTS: rate - the desired rate for rtc
 *   OUTPUTS: 0 on success, -1 on failure
 */
int rtc_set_rate(int rate) {
    unsigned int flags;
    char prev;

    //sanity check
    if (rate < 6 || rate > RATE_1024_HZ) { //when rate = 6, frequency = 1024Hz
        return -1;
    }
    
    //set frequency		
    cli_and_save(flags);
    outb(register_A, RTC_INDEX);		// set index to register A, disable NMI
    prev=inb(CMOS);	                    // get initial value of register A
    outb(register_A, RTC_INDEX);	    // reset index to A
    outb((prev & 0xF0) | rate, CMOS);   // write only our rate to A. Note, rate is the bottom 4 bits. 0xF0 = 1111 0000
    restore_flags(flags);

    return 0;
}
