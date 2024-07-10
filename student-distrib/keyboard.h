#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#ifdef __GNUC__
#define VARIABLE_IS_NOT_USED __attribute__ ((unused))
#else
#define VARIABLE_IS_NOT_USED
#endif

#include "i8259.h"
#include "lib.h"
#include "terminal.h"
#include "multi_term.h"

/* scancode table size is 0x59
 * the largest scancode is 0x58
 * add one empty code 0 at front to make indexing easier
 */
#define SCANCODE_SIZE   0x59

/* define the special keyboard inputs that can trigger a flag change*/
#define backspace_pressed 0x0E
#define tab_pressed 0x0F
#define enter_pressed 0x1C
#define left_control_pressed 0x1D
#define left_control_released 0x9D
#define left_shift_pressed 0x2A
#define left_shift_released 0xAA
#define right_shift_pressed 0x36
#define right_shift_released 0xB6
#define left_alt_pressed 0x38
#define caps_lock_pressed 0x3A
#define right_control_pressed 0x1D
#define right_control_released 0x9D
#define l_pressed 0x26
#define c_pressed 0x2E
#define l_released 0xA6
#define alt_pressed 0x38
#define alt_released 0xB8
#define f1_pressed 0x3B
#define f2_pressed 0x3C
#define f3_pressed 0x3D
#define f1_released 0xBB
#define f2_released 0xBC
#define f3_released 0xBD


// size of buffer is 128
#define kbuf_size 128

// number of terminals supported
#define nterms 3

// for scancode not printed
#define NULL_CHAR 0x00

// scan code read from user
unsigned char scan_code;

// keyboard buffers to store user input
char kbuf_entered[nterms][kbuf_size];
// the temporary buffers used to store input before an enter is pressed
char kbuf[nterms][kbuf_size];

//tracking the size of buffer
int cur_kbuf_size[nterms];

//special condition flags
int caps_lock;
int shift;
int control;
int control_l;
int control_c;
int enter_flag;
int alt;
int alt_f1;
int alt_f2;
int alt_f3;

//enable IRQ for keyboard
void init_keyboard();

//keyboard handler function, read and echo the key pressed
void keyboard_handler();

//update flags for shift, control, etc
int update_flags(unsigned char scan_code);

//get the correct key to print depending on flags
char get_key(unsigned char scan_code);

//print the key and update buffer
void print_key(char key, unsigned char scan_code);

//clear the screen when ctrl l is pressed
void clear_screen();

//halt current program when ctrl c is pressed
void halt_program();

//call terminal_switch functions in terminal.c if needed
void switch_terminal();

#endif
