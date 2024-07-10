#include "multi_term.h"

//setting up terminal 0 shell upon bootup
int32_t init_terminal(int idx) {
        

        if(idx >= 0 && idx < 3){
            cli();
            int j;
            for(j = 0; j < KBUF_SIZE; j++){
                terminal[idx].kbuf[j] = NULL_CHAR;
                terminal[idx].kbuf_entered[j] = NULL_CHAR;
            }

            terminal[idx].cur_kuf_size = 0;
            terminal[idx].terminal_screen_x= 7;
            terminal[idx].terminal_screen_y= 1;

            pcb_t* curr_pcb = get_pcb(schedule[active_term_idx]);
            if(idx != 0){
                uint32_t ebp;
                asm volatile ("movl %%ebp, %0\n" :"=r"(ebp));
                curr_pcb->kernel_ebp = ebp;
                uint32_t esp;
                asm volatile ("movl %%esp, %0\n" :"=r"(esp));
                curr_pcb->kernel_esp = esp;
            }
            
            active_term_idx = idx; //update active term idx
            execute((uint8_t*)"shell");
            sti();
        }
    return 0;

}


int32_t terminal_switch(int32_t from_idx, int32_t to_idx) {
    cli();

    // store cursor position of terminal from_idx
    terminal[from_idx].terminal_screen_x = get_x();
    terminal[from_idx].terminal_screen_y = get_y();

    // update screen_x and screen_y to the cursor position of terminal to_idx
    set_x(terminal[to_idx].terminal_screen_x);
    set_y(terminal[to_idx].terminal_screen_y);

    update_cursor(terminal[to_idx].terminal_screen_x, terminal[to_idx].terminal_screen_y);

    //update video page
    //save current terminal screen to video page assigned to it
    memcpy((uint8_t *)(VID_PAGE_T0 + (from_idx * PAGE_SIZE_4KB)), (uint8_t *) VIDEO_MEMORY_START, PAGE_SIZE_4KB);
    //restore new terminal screen to video memory
    memcpy((uint8_t *) VIDEO_MEMORY_START, (uint8_t *)(VID_PAGE_T0 + (to_idx * PAGE_SIZE_4KB)), PAGE_SIZE_4KB);

    sti();
    return 0;
}


