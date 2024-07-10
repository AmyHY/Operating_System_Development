#include "keyboard.h"

#define KEYBOARD_IRQ       0x01
#define KEYBOARD_PORT      0x60

//lookup table for converting scancode to the key pressed
// used scancode to index table
//Referenced from: https://www.plantation-productions.com/Webster/www.artofasm.com/DOS/pdf/apndxc.pdf 

static char scancode_lookup[SCANCODE_SIZE] = {
    '\x00',            // empty code
    '\x00',            // escape
    '1',
    '2',
    '3',
    '4',
    '5',
    '6',
    '7',
    '8',
    '9',
    '0',
    '-',            
    '=',
    '\x00',
    '\t',
    'q',            
    'w',
    'e',
    'r',
    't',
    'y',            //15
    'u',
    'i',
    'o',
    'p',
    '[',
    ']',
    '\n',
    '\x00',
    'a',
    's',            //1f
    'd',
    'f',
    'g',
    'h',
    'j',
    'k',
    'l',
    ';',
    '\'',
    '`',            //29
    '\x00',
    '\\',
    'z',
    'x',
    'c',
    'v',
    'b',
    'n',
    'm',
    ',',            //33
    '.',
    '/',
    '\x00',
    '*',
    '\x00',
    ' ',            //space 
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',            //7
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00',
    '\x00'
};


/* 
 * init_keyboard
 *   DESCRIPTION: enable (unmask) IRQ for keyboard
 *   INPUTS: none
 *   OUTPUTS: none
 */

void init_keyboard(){
    enable_irq(KEYBOARD_IRQ);
    // iterate the buffer to update everything to null
    // int i;
    // int j;
    
    // for (j=0; j<nterms; j++) {
    //     // initialize buffers for each terminal
    //     for (i=0; i<kbuf_size; i++) {
    //         kbuf[j][i] = NULL_CHAR;
    //         kbuf_entered[j][i] = NULL_CHAR;
    //     }

    //     // current buffer size = 0 for each terminal
    //     cur_kbuf_size[j] = 0;

    //}

    // intialize flags for each terminal
    caps_lock = 0;
    shift = 0;
    control = 0;
    control_l = 0;
    control_c = 0;
    enter_flag = 0;
    alt = 0;
    alt_f1 = 0;
    alt_f2 = 0;
    alt_f3 = 0;

}

/* 
 * keyboard_handler
 *   DESCRIPTION: reads in scancode from keyboard port and echo/print 
 *   the key pressed onto the screen
 *   INPUTS: none
 *   OUTPUTS: none
 */
void keyboard_handler(){
    char key;
    int flag_updated;
    //read scan code
    scan_code = inb(KEYBOARD_PORT);
    //update flags
    flag_updated = update_flags(scan_code);
    //clear screen if control l flag is on
    clear_screen();
    //call halt if ctrl_c is pressed
    halt_program();
    // switch terminal if alt f is pressed
    switch_terminal(); 
    // print key only if the key is printable
    if (flag_updated == 0 && scan_code < SCANCODE_SIZE) {
        key = get_key(scan_code);
        print_key(key, scan_code);            
    }
    //send eoi to signal end 
    send_eoi(KEYBOARD_IRQ);
}

/* 
 * update_flags
 *   DESCRIPTION: update the control shift etc flags 
 *   INPUTS: scan_code
 *   OUTPUTS: 0 if no flags updated, 1 if some flag updated
 */
int update_flags(unsigned char scan_code) {
    // update shift
    if (scan_code == left_shift_pressed || scan_code ==right_shift_pressed) {
        shift = 1;
        return 1;
    }

    if (scan_code == left_shift_released || scan_code == right_shift_released) {
        shift = 0;
        return 1;
    }

    // update caps_lock
    if (scan_code == caps_lock_pressed) {
        caps_lock = 1 - caps_lock;
        return 1;
    }

    // update control
    if (scan_code == left_control_pressed || scan_code ==right_control_pressed) {
        control = 1;
        return 1;
    }

    if (scan_code == left_control_released || scan_code == right_control_released) {
        control = 0;
        return 1;
    }

    // update control l
    if (scan_code == l_pressed && control == 1) {
        control_l = 1;
        return 1;
    }

    // update control c
    if (scan_code == c_pressed && control == 1) {
        control_c = 1;
        return 1;
    }

    // update alt
    if (scan_code == alt_pressed) {
        alt = 1;
        return 1;
    }

    if (scan_code == alt_released) {
        alt = 0;
        return 1;
    }

    // update control alt_f
    if (scan_code == f1_pressed && alt == 1) {
        alt_f1 = 1;
        return 1;
    }

    if (scan_code == f2_pressed && alt == 1) {
        alt_f2 = 1;
        return 1;
    }

    if (scan_code == f3_pressed && alt == 1) {
        alt_f3 = 1;
        return 1;
    }


    // nothing updated, return 0
    return 0;

}

/* 
 * clear_screen
 *   DESCRIPTION: clear screen if control l is pressed 
 *   INPUTS: none
 *   OUTPUTS: none
 */
void clear_screen() {
    if (control_l == 1) {
        clear();
        // restore control l flag
        control_l = 0;

        puts("Starting 391 Shell\n");
        // printf("Terminal %d\n", current_terminal_idx);
        puts("391OS> ");

        // clean buffer
        int i;
        for (i = 0; i < kbuf_size; i++) {
           // kbuf[current_terminal_idx][i] = NULL_CHAR;
           terminal[visible_term_idx].kbuf[i] = NULL_CHAR;
        }
        //cur_kbuf_size[current_terminal_idx] = 0;
        terminal[visible_term_idx].cur_kuf_size = 0;
    }
}

void halt_program(){
    if(control_c == 1 && (active_term_idx == visible_term_idx)){
        control_c = 0;
        send_eoi(KEYBOARD_IRQ);
        halt(0);
    }
}

/* 
 * switch_terminal
 *   DESCRIPTION: call terminal_switch functions in terminal.c if needed
 *   INPUTS: none
 *   OUTPUTS: none
 */
void switch_terminal() {
    // switch to terminal 0
    if (alt_f1 == 1 && visible_term_idx != 0) {
        alt_f1 = 0;
        
        send_eoi(KEYBOARD_IRQ);
        terminal_switch(visible_term_idx, 0);
        //update current visible terminal idx
        visible_term_idx = 0;
        //puts("Switch to terminal 0\n");

    }
    // switch to terminal 1
    if (alt_f2 == 1 && visible_term_idx != 1) {
        alt_f2 = 0;
       
        send_eoi(KEYBOARD_IRQ);
        terminal_switch(visible_term_idx, 1);
        //update current visible terminal idx
        visible_term_idx = 1;
        //puts("Switch to terminal 1\n");
    
    }
    // switch to terminal 2
    if (alt_f3 == 1 && visible_term_idx != 2) {
        alt_f3 = 0;
    
        send_eoi(KEYBOARD_IRQ);
        terminal_switch(visible_term_idx, 2);
        //update current visible terminal idx
        visible_term_idx = 2;
        //puts("Switch to terminal 2\n");
        
    }
    
}

/* 
 * get_key
 *   DESCRIPTION: get the key depending on shift and caps_lock
 *   INPUTS: scan_code
 *   OUTPUTS: the desired key
 */
char get_key(unsigned char scan_code) {
    char ret_key;
    // access the original key with scan_code
    ret_key = scancode_lookup[scan_code];

    // if caps_lock or shift on, give capitalized letter
    if (caps_lock == 1 || shift == 1) {
        if (ret_key == 'a') ret_key = 'A';
        if (ret_key == 'b') ret_key = 'B';
        if (ret_key == 'c') ret_key = 'C';
        if (ret_key == 'd') ret_key = 'D';
        if (ret_key == 'e') ret_key = 'E';
        if (ret_key == 'f') ret_key = 'F';
        if (ret_key == 'g') ret_key = 'G';
        if (ret_key == 'h') ret_key = 'H';
        if (ret_key == 'i') ret_key = 'I';
        if (ret_key == 'j') ret_key = 'J';
        if (ret_key == 'k') ret_key = 'K';
        if (ret_key == 'l') ret_key = 'L';
        if (ret_key == 'm') ret_key = 'M';
        if (ret_key == 'n') ret_key = 'N';
        if (ret_key == 'o') ret_key = 'O';
        if (ret_key == 'p') ret_key = 'P';
        if (ret_key == 'q') ret_key = 'Q';
        if (ret_key == 'r') ret_key = 'R';
        if (ret_key == 's') ret_key = 'S';
        if (ret_key == 't') ret_key = 'T';
        if (ret_key == 'u') ret_key = 'U';
        if (ret_key == 'v') ret_key = 'V';
        if (ret_key == 'w') ret_key = 'W';
        if (ret_key == 'x') ret_key = 'X';
        if (ret_key == 'y') ret_key = 'Y';
        if (ret_key == 'z') ret_key = 'Z';
    }
    // if shift is on give upper input
    if (shift == 1) {
        if (ret_key == '`') ret_key = '~';
        if (ret_key == '1') ret_key = '!';
        if (ret_key == '2') ret_key = '@';
        if (ret_key == '3') ret_key = '#';
        if (ret_key == '4') ret_key = '$';
        if (ret_key == '5') ret_key = '%';
        if (ret_key == '6') ret_key = '^';
        if (ret_key == '7') ret_key = '&';
        if (ret_key == '8') ret_key = '*';
        if (ret_key == '9') ret_key = '(';
        if (ret_key == '0') ret_key = ')';
        if (ret_key == '-') ret_key = '_';
        if (ret_key == '=') ret_key = '+';
        if (ret_key == '[') ret_key = '{';
        if (ret_key == ']') ret_key = '}';
        if (ret_key == '\\') ret_key = '|';
        if (ret_key == ';') ret_key = ':';
        if (ret_key == '\'') ret_key = '"';
        if (ret_key == ',') ret_key = '<';
        if (ret_key == '.') ret_key = '>';
        if (ret_key == '/') ret_key = '?';
    }
    
    return ret_key; 
}

/* 
 * print_key
 *   DESCRIPTION: print the key if printable and update buffer
 *   INPUTS: key, scan_code
 *   OUTPUTS: none
 */
void print_key(char key, unsigned char scan_code) {
    // handle backspace
    if (scan_code == backspace_pressed) {
        // should only delete if something is in buffer
        if (terminal[visible_term_idx].cur_kuf_size > 0) {
            // deleting a tab
            //kbuf[current_terminal_idx][cur_kbuf_size[current_terminal_idx]-1] == '\t'
            if (terminal[visible_term_idx].kbuf[terminal[visible_term_idx].cur_kuf_size - 1] == '\t') {
                rmc();
                rmc();
                rmc();
                rmc();
            } else {
                rmc();
            }
            // update buffer
            // kbuf[current_terminal_idx][cur_kbuf_size[current_terminal_idx]] = NULL_CHAR;
            // cur_kbuf_size[current_terminal_idx]--;
            terminal[visible_term_idx].kbuf[terminal[visible_term_idx].cur_kuf_size] = NULL_CHAR;
            terminal[visible_term_idx].cur_kuf_size--;
        }
    }
    // cases other than backspace
    if(key != NULL_CHAR){ 
        putc(key);  //print the char to screen

        //TODO check this logic
        if ((scan_code == enter_pressed)) {
            //when enter is pressed
            terminal[visible_term_idx].kbuf[terminal[visible_term_idx].cur_kuf_size] = '\n';

            // clean the tmp buffer and enter the keyboard buffer
            int i;
            for (i = 0; i < kbuf_size; i++) {
                terminal[visible_term_idx].kbuf_entered[i] = terminal[visible_term_idx].kbuf[i];
                terminal[visible_term_idx].kbuf[i] = NULL_CHAR;
            }

            terminal[visible_term_idx].cur_kuf_size = 0;
            terminal[visible_term_idx].enter_flag = 1;       //indicate command has been entered

        } else if (scan_code != enter_pressed && terminal[visible_term_idx].cur_kuf_size < (kbuf_size - 1)) {
            // print an ordinary character

            terminal[visible_term_idx].kbuf[terminal[visible_term_idx].cur_kuf_size] = key;
            terminal[visible_term_idx].cur_kuf_size++;
        }
    }
}
