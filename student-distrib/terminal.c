#include "terminal.h"

/* terminal_read;
 * Inputs: fd - not used for ckpt2
 *         buf - pointer to the user buffer to modify
 *         nbytes - the max number of characters to read
 * Return Value: read_num - number of characters successfully read
 * Function: Read from keyboard buffer 
 * returns only when enter key is pressed and should always add /n before return
 * able to handler buffer overflow*/
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes) {
    //printf("read: %d, %d\n", active_term_idx, visible_term_idx);
    
    // cast to char type in order to modify input
    char* char_buf = (char*)buf;
    // return value
    int32_t read_num = 0;
    read_num = 0;

    if (char_buf == NULL) {
        return read_num;
    }

    //TODO check this logic
    //if(visible_term_idx == active_term_idx){
        //wait until user had input something
        while(1){
            if(terminal[visible_term_idx].enter_flag == 1 && (visible_term_idx == active_term_idx)){
                break;
            }
        }
    
        int i;
        // iterating the keyboard buffer
        for (i = 0; i < nbytes; i++) {
            //last char in the buf is always '\n'
            if(i == (kbuf_size - 1)){
                char_buf[i] = '\n';
                read_num++;
                break;
            }

            //char_buf[i] = kbuf_entered[current_terminal_idx][i];
            char_buf[i] = terminal[visible_term_idx].kbuf_entered[i];
            read_num++;

            // end reading at newline character
            if (char_buf[i] == '\n') {
                break;
            }
        }

        terminal[visible_term_idx].enter_flag = 0;
    //}
    return read_num;
}

/* terminal_write;
 * Inputs: fd - not used for ckpt2
 *         buf - pointer to the user buffer to write
 *         nbytes - the max number of characters to write
 * Return Value: write_num - number of characters successfully written
 * Function: Write to terminal */
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes) {
    //printf("write: %d, %d\n", active_term_idx, visible_term_idx);
    // cast to char type in order to access value
    char* char_buf = (char*)buf;
    // return value
    int32_t write_num = 0;

    //what we are seeing is not the active process
    //write to backup mem
    
    cli();

    if (char_buf == NULL) {
        return write_num;
    }

    // iterating the user buffer
    int i;
    for (i = 0; i < nbytes; i++) { 
        // skip null characters
        if ((char_buf[i] != NULL_CHAR)) {
            // printf("terminal_index:%d,%d\n", active_term_idx, visible_term_idx);
            if( active_term_idx != visible_term_idx){
                // printf("terminal_index:%d,%d\n", active_term_idx, visible_term_idx);
                putc_diff(char_buf[i], active_term_idx);
                //putc(char_buf[i]);
            }
            else{
                putc(char_buf[i]);
            }
                write_num++; 
        }
        
    }

    sti();
    //update_cursor(terminal[active_term_idx].terminal_screen_x, terminal[active_term_idx].terminal_screen_y);
    return write_num;
}

/* terminal_open;
 * Inputs: filename - not used for ckpt2
 * Return Value: 0
 * Function: do nothing in ckpt2 */
int32_t terminal_open(const uint8_t* filename) {
    return 0;
}

/* terminal_c;ose;
 * Inputs: fd - not used for ckpt2
 * Return Value: 0
 * Function: do nothing in ckpt2 */
int32_t terminal_close(int32_t fd) {
    return 0;
}
