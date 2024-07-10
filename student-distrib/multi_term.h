#ifndef _MULTI_TERM_H
#define _MULTI_TERM_H

#include "types.h"
#include "lib.h"
#include "syscall.h"
#include "page.h"

// current terminal
#define NTERMS          3           //this supports max of 3 terminals        
#define KBUF_SIZE       128     
// for scancode not printed
#define NULL_CHAR 0x00    

int visible_term_idx; //visible terminal
int active_term_idx; //terminal that has the active process
int init_idx;
//struct that holds info about each terminal
typedef struct term{
    int terminal_screen_x;
    int terminal_screen_y;

    char kbuf_entered[KBUF_SIZE];
    char kbuf[KBUF_SIZE];
    int cur_kuf_size;
    int enter_flag;

    int max_rtc_count;
    int rtc_count;
} terminal_t;

terminal_t terminal[NTERMS];

//setting up terminal 0 shell upon bootup
int32_t init_terminal(int idx);

//setup terminal 1 and 2 shell when alt + f# is pressed for the first time
//void launch_new_terminal(int idx)

//switches the terminal visible 
int32_t terminal_switch(int32_t from_idx, int32_t to_idx);



#endif /* _MULTI_TERM_H */
