#include "pit.h"


void init_pit(){
    int reload_val;
    reload_val = INPUT_FREQ/PIT_FREQ;

    outb(PIT_MODE, PIT_CMD_REG);
    outb(reload_val & 0xFF, PIT_CH0);   //moving lower byte
    outb(reload_val >> 8, PIT_CH0);     //moving higher byte

    active_term_idx = 0;
    visible_term_idx = 0;

    i = -1;
    
    int j;
    for(j = 0; j < NTERMS; j++){
        schedule[j] = -1;
    }

    enable_irq(PIT_IRQ);
}

void pit_handler(){
   //printf("hi");
   send_eoi(PIT_IRQ);
   
    if(i < 2){
        i++;
        init_terminal(i);
    }

    else{
        //printf("switch pit handler%d\n", active_term_idx);
        
        int old_term_idx = active_term_idx;
        active_term_idx = (active_term_idx + 1)%3;  //get the next scheduled terminal idx

        cli();
        process_switch(schedule[old_term_idx],schedule[active_term_idx]);
        sti();
            
    }
    
    

}


