#ifndef TESTS_H
#define TESTS_H

// test launcher
void launch_tests();

//tests idt launch and entries
int idt_test();
	
//test exceptions
void div_by_zero_test();
int page_fault_test(); //needs paging to be initialized first

//test receiving an rtc interrupt
int rtc_interrupt_test();

//test interpreting various scancodes in the keyboard handler
int keyboard_scancode_test();

//checks garbage input in CheckPoint 1
int garbage_input_test();

//checks correct return value
int return_value_test();

//test rtc_open function
int rtc_open_test();

//test rtc_read function
int rtc_read_test();

//test rtc_write function
int rtc_write_test();

//test rtc_close function
int rtc_close_test();

//test rtc_set_rate function
int rtc_set_rate_test();

//checks rtc garbage input in CheckPoint 2
int rtc_garbage_input_test();
// check reading from user and writing to terminal
void keyboard_terminal_interactive_test(int write_nbytes);

// check if terminal write handles null input
int keyboard_terminal_null_test();

// test if we can read a file and print to stdout.
int file_test(char* file_name, int file_size); 

#endif /* TESTS_H */
