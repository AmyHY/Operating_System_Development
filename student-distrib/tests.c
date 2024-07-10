#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"
#include "rtc.h"
#include "keyboard.h"
#include "page.h"
#include "filesystem.h"
#include "terminal.h"

#define PASS 1
#define FAIL 0

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test - Example
 * 
 * Asserts that first 20 IDT entries (except 15th), the sys_call, 
 * the keyboard int, and the rtc int entries are not NULL and have
 * valids values in their fields
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;

	int i;
	int result = PASS;
	//check the first 10 exceptions
	for (i = 0; i <10; ++i) {
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}
	asm volatile ("int $0x2"); //plug in different idt# to test exv=ceptions
	return result;
}

// Test access of video memery in the first 4MB of virtual memory (expect PASS)
int video_memory_access_test() {
    TEST_HEADER;
    char value;
    value = *(char*)0xB8000; // Test access at the start of video memory
    value = *(char*)0xBC000; // Test access at the middle of video memory
    value = *(char*)0xBFFFF; // Test access at the end of video memory
	return PASS;
}

// Test invalid access to memory in the first 4MB of virtual memory other than the video memory (expect page fault)
int page_table_invalid_access_test() {
    TEST_HEADER;
    char value = *(char*)0x30000; // Not present in physical memory (video memory starts at 0x48000)
	value = value + 1; // Avoid unused variable warning.
	return PASS;
}

// Test access to the kernel code in 4MB to 8MB in virtual memory (expect PASS)
int kernel_code_access_test() {
    TEST_HEADER;
    char value = *(char*)0x400001; // Kernel memory starts from 0x400000
	value = value + 1; // Avoid unused variable warning.
	return PASS;
}

// Test access to 4MB pages other than the first two. (expect page fault)
int other_4MB_pages_access_test() {
    TEST_HEADER;
    char value = *(char*)0x800001; // The third 4MB page (8MB - 12MB) should not be initialized.
	value = value + 1; // Avoid unused variable warning.
	return PASS;
}

// Test access to the image of the user program (expect PASS)
int user_image_access_test() {
    TEST_HEADER;
    char value = *(char*)0x08048000; // The third 4MB page (8MB - 12MB) should not be initialized.
	value = value + 1; // Avoid unused variable warning.
	return PASS;
}

// Test access to the null pointer (expect page fault)
int null_pointer_access_test() {
    TEST_HEADER;
    char value = *(char*)0x0;
	value = value + 1; // Avoid unused variable warning.
	return PASS;
}

// Check the value contained in the page directory entry is equal to the original set value, except for the changes in the dirty and accessed bits. (Expect PASS)
int page_directory_value_test() {
	TEST_HEADER;
	if (page_directory[1] == 0x4000E9) {
		return PASS;
	}
	return FAIL;
}

// Check the value contained in the page table entry for the start of video memory is equal to the original set value, except for the changes in the dirty and accessed bits. (Expect PASS)
int page_table_value_test() {
	TEST_HEADER;
	// 184 is the index of the start of the video memory (0xB8000 / 4096 = 184)
	int value = *(int*)&(page_table[184]);
	if (value == 0xB806B) {
		return PASS;
	}
	return FAIL;
}

/* Keyboard Terminal Interactive Test
 * 
 * Test if terminal_read can read user input and terminal write can print to screen
 * Input write_nbytes define the max number of characters 
 * terminal_write want to copy and print
 * Inputs: write_nbytes
 * Outputs: None
 * Side Effects: None
 * Coverage: terminal_read, terminal_write, keyboard_handler
 * Files: keyboard.c/h, terminal.c/h
 */
void keyboard_terminal_interactive_test(int write_nbytes) {
	int32_t read_num, write_num;
	char test_buffer[kbuf_size];

	clear();

	printf("Type something and press enter.\n");
	printf("You can enter at most %d characters but the terminal will only write the first %d of them.\n", kbuf_size, write_nbytes);
	printf("This number also includes the new line character.\n");

	printf("R: ");
	// wait until the user presses enter to start reading
	while (scan_code != enter_pressed);
	read_num = terminal_read(0, test_buffer, kbuf_size);

	printf("W: ");
	write_num = terminal_write(0, test_buffer, write_nbytes);

	printf("Read %d characters from user and write %d characters to the terminal\n", read_num, write_num);

}

/* Keyboard Terminal Interactive Test
 * 
 * Test if terminal write can handle buffer with null characters inside
 * Should ignore null character when printing and counting write_num
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: terminal_write, keyboard_handler
 * Files: keyboard.c/h, terminal.c/h
 */
int keyboard_terminal_null_test() {
	TEST_HEADER;
	int32_t write_num;
	// make a buffer with null character in it
	char test_buffer[kbuf_size] = {'E', '\t', 'C', NULL_CHAR, 'E', NULL_CHAR, '\n'};

	printf("W: ");
	write_num = terminal_write(0, test_buffer, kbuf_size);

	printf("Write %d characters to the terminal\n", write_num);

	// should ignore the null characters
	if (write_num == 5) {
		return PASS;
	}
	return FAIL;
}


/* div_by_zero_test
 * 
 * throws a Divide Error Exception
 * Inputs: None
 * Outputs: None
 * Side Effects: None
 * Coverage: set_exceptions, init_idt
 * Files: none
 */
void div_by_zero_test() {
	TEST_HEADER;
	int a,b;
	a = 0;
	b = 1/a;
}

// void overflow_test(){
// 	TEST_HEADER;
// 	char a = 0xff;
// 	char b = 0xff;
// 	a = a + b;
// }

// add more tests here


/* page_fault_test
 * 
 * throws a Page-Fault Exception
 * Inputs: None
 * Outputs: int
 * Side Effects: None
 * Coverage: set_exceptions, init_idt
 * Files: none
 */
int page_fault_test() { 
	//tests dereferencing diff address ranges with paging turned on 
	TEST_HEADER;
	int* ptr;
	*ptr = NULL;
	int a;
	a = *(ptr);
	return a;
}


/* rtc_interrupt_test
 * 
 * tests receiving an rtc interrupt
 * Inputs: None
 * Outputs: 1 on success
 * Side Effects: None
 * Coverage: rtc_handler
 * Files: none
 */
int rtc_interrupt_test() { //remember to uncomment "test_interrupt" in rtc handler
	TEST_HEADER;
	rtc_handler(); // the screen should be spamming diff images.
	return 1;
}


/* keyboard_scancode_test
 * 
 * tests interpreting various scancodes in the keyboard handler
 * Inputs: None
 * Outputs: 1 on success
 * Side Effects: None
 * Coverage: keyboard_handler
 * Files: none
 */
int keyboard_scancode_test() {
	TEST_HEADER;
	keyboard_handler();
	return 1;
}


/* garbage_input_test
 * 
 * this should not disable anything
 * Inputs: None
 * Outputs: 1 on success
 * Side Effects: None
 * Coverage: disable_irq
 * Files: none
 */
int garbage_input_test() { //should not disable anything
	TEST_HEADER;
	disable_irq(17); // 17 is out of IRQ bounds 0-15
	return 1;
}


/* return_value_test
 * 
 * this should disable keyboard
 * Inputs: None
 * Outputs: 1 on success
 * Side Effects: None
 * Coverage: disable_irq
 * Files: none
 */
int return_value_test() { //should disable keyboard
	TEST_HEADER;
	disable_irq(0x01); //0x01 is keyboard IRQ
	return 1;
}

/* 
 * file_open_test
 *   DESCRIPTION: test if we can successfully open a file or a directory.
 *   INPUTS: None
 *   OUTPUTS: return PASS if can open a file, return FAIL otherwise.
 */
int file_open_test() {
	char* file_name = "fish";
	if (file_open((uint8_t*)file_name) != 0) {
		return FAIL;
	}
	char* dir_name = ".";
	if (dir_open((uint8_t*)dir_name) != 0) {
		return FAIL;
	}
	return PASS;
}

/* 
 * file_test_medium
 *   DESCRIPTION: test if we can read a medium file and print to stdout.
 *   INPUTS: None
 *   OUTPUTS: return PASS if successfully got results.
 */
int file_test_medium() {
	// char* file_name = "ls";
	// file_open((uint8_t*)file_name);
	// int file_size = 5349;
	char* file_name = "grep";
	file_open((uint8_t*)file_name);
	int file_size = 6149;
	char buf[file_size];
	// Read file content into buf.
	file_read(0, &buf, file_size);
	int i;
	// Print characters each at a time.
	for (i = 0; i < file_size; i++) {
		putc(buf[i]);
	}
	file_close(0);
	return PASS;
}


/* 
 * file_test_hard
 *   DESCRIPTION: test if we can read a large file and print to stdout.
 *   INPUTS: None
 *   OUTPUTS: return PASS if successfully got results.
 */
int file_test_hard() {
	char* file_name = "verylargetextwithverylongname.txt";
	file_open((uint8_t*)file_name);
	int file_size = 5277;
	// char* file_name = "shell";
	// file_open((uint8_t*)file_name);
	// int file_size = 5605;
	char buf[file_size];
	// Read file content into buf.
	file_read(0, &buf, file_size);
	int i;
	// Print characters each at a time.
	for (i = 0; i < file_size; i++) {
		putc(buf[i]);
	}
	file_close(0);
	return PASS;
}

/* 
 * file_test
 *   DESCRIPTION: test if we can read a file and print to stdout.
 *   INPUTS: None
 *   OUTPUTS: return PASS if successfully got results.
 */
int file_test(char* file_name, int file_size) {
	file_open((uint8_t*)file_name);
	char buf[file_size];
	// Read file content into buf.
	file_read(0, &buf, file_size);
	int i;
	// Print characters each at a time.
	for (i = 0; i < file_size; i++) {
		putc(buf[i]);
	}
	file_close(0);
	return PASS;
}



/* 
 * file_write
 *   DESCRIPTION: test if both file_write and dir_write return -1 all the times.
 *   INPUTS: None
 *   OUTPUTS: return PASS if successfully both results got -1, return FAIL otherwise.
 */
int file_write_test() {
	char* buf;
	if (file_write(0, buf, 1) != -1) {
		return FAIL;
	}
	if (dir_write(0, buf, 1) != -1) {
		return FAIL;
	}
	return PASS;
}

/* 
 * file_write_hard
 *   DESCRIPTION: test if closing invalid file descriptor will return -1, and closing valid ones will return 0.
 *   INPUTS: None
 *   OUTPUTS: return PASS if behaving as expected, return FAIL otherwise.
 */
int file_close_test() {
	if (file_close(0) != -1) {
		return FAIL;
	}
	if (dir_close(2) != 0) {
		return FAIL;
	}
	return PASS;
}
/* 
 * partial_read_file
 *   DESCRIPTION: test if we can read partial parts from the the file.
 *   INPUTS: None
 *   OUTPUTS: return PASS if read provided bytes, return FAIL otherwise.
 */
int partial_read_file() {
	char* file_name = "frame1.txt";
	// File size equals to 174
	file_open((uint8_t*)file_name);
	// Only read the first 100 bytes out of 174 bytes in the file.
	int bytes_to_read = 100;
	char buf[bytes_to_read];
	// Read file content into buf.
	int read_bytes = file_read(0, &buf, bytes_to_read);
	// Check if we have read bytes equals to number of bytes we intend.
	if (read_bytes != bytes_to_read) {
		return FAIL;
	}
	int i;
	// Print characters each at a time.
	for (i = 0; i < bytes_to_read; i++) {
		putc(buf[i]);
	}
	file_close(0);
	return PASS;
}

/* 
 * read_file_out_of_length
 *   DESCRIPTION: test if we can read file appropritely when the request length is larger than the file size.
 *   INPUTS: None
 *   OUTPUTS: return PASS if the return value is correct, return FAIL otherwise.
 */
int read_file_out_of_length() {
	char* file_name = "frame1.txt";
	file_open((uint8_t*)file_name);
	int file_size = 174;
	// We try to read the 200 bytes, but only 174 bytes in the file.
	int bytes_to_read = 200;
	char buf[bytes_to_read];
	// Read file content into buf.
	int read_bytes = file_read(0, &buf, bytes_to_read);
	// If we try to read more bytes than number of bytes in the file, return value of 0 is expected.
	if (read_bytes != 0) {
		return FAIL;
	}
	int i;
	// Print characters each at a time.
	for (i = 0; i < file_size; i++) {
		putc(buf[i]);
	}
	file_close(0);
	return PASS;
}


/* 
 * dir_test
 *   DESCRIPTION: test if we can read file name from a directory along with the file type and length.
 *   INPUTS: None
 *   OUTPUTS: return PASS if successfully got results.
 */
int dir_test() {
	// Open the directory for future operations.
	char* dir_name = ".";
	dir_open((uint8_t*) dir_name);
	while (1) {
		char buf[MAX_FILENAME_LEN];
		// Get the dentry to retrieve information about the current file.
		dentry_t curr_dentry;
		read_dentry_by_index(info.counter, &curr_dentry);
		inode_t* inode_ptr = info.inode_start + curr_dentry.inode_num;
		// If we have read all files.
		if (dir_read(0, &buf, MAX_FILENAME_LEN) == 0) {
			break;
		}
		// Get the file name length and append NULL byte to end the string.
		uint32_t len = strlen(buf);
		if (len >= MAX_FILENAME_LEN) {
			len = MAX_FILENAME_LEN;
		}
		buf[len] = '\0';
		printf("file name: %s, file type: %d, file size: %d\n", buf, curr_dentry.filetype, inode_ptr->length);
	}
	return PASS;
}

/* Checkpoint 2 tests */


/* rtc_open_test
 * 
 * tests if rtc_open returns 0 on valid arg
 * Inputs: None
 * Outputs: 1 on success
 * Side Effects: None
 * Coverage: rtc_open
 * Files: none
 */
int rtc_open_test() { 
	TEST_HEADER;
	uint8_t random_int = 1;
	uint8_t* filename = &random_int;
	if (rtc_open(filename) == 0) {
		return 1; //pass
	}
	return 0; //else fail
}


/* rtc_read_test
 * 
 * tests if rtc_read returns 0 on valid arg
 * Inputs: None
 * Outputs: 1 on success
 * Side Effects: None
 * Coverage: rtc_read
 * Files: none
 */
int rtc_read_test() { 
	TEST_HEADER;
	int32_t fd = 1;
	int two_hertz = 2;
	int* buf = &two_hertz;
	int32_t nbytes = 4;
	if (rtc_read(fd, buf, nbytes) == 0) {
		return 1; //pass
	}
	return 0; //else fail
}


/* rtc_write_test
 * 
 * tests if rtc_write returns number of bytes written 
 * on valid arg, as well as setting the screen to the 
 * desired spamming frquency
 * Inputs: None
 * Outputs: 1 on success
 * Side Effects: None
 * Coverage: rtc_write
 * Files: none
 */
int rtc_write_test() { 
	TEST_HEADER;
	int32_t fd = 1;
	int random_hertz = 1024; //a random # of 2's power
	int* buf = &random_hertz;
	int32_t nbytes = 4;
	if (rtc_write(fd, buf, nbytes) == nbytes) {
		return 1; //pass
	}
	return 0; //else fail
}


/* rtc_close_test
 * 
 * tests if rtc_close returns 0 on valid arg
 * Inputs: None
 * Outputs: 1 on success
 * Side Effects: None
 * Coverage: rtc_close
 * Files: none
 */
int rtc_close_test() { 
	TEST_HEADER;
	int32_t fd = 1;
	if (rtc_close(fd) == 0) {
		return 1; //pass
	}
	return 0; //else fail
}



/* rtc_set_rate_test
 * 
 * tests if rtc_set_rate changes the screen to
 * the desired spamming frequency
 * Inputs: None
 * Outputs: 1 on success
 * Side Effects: None
 * Coverage: rtc_set_rate
 * Files: none
 */
int rtc_set_rate_test() {
	TEST_HEADER;
	int rate = 1024; //random # of 2's power
	if (rtc_set_rate(rate) == 0) {
		return 1; //pass
	}
	return 0; //else fail
}


/* rtc_garbage_input_test
 * 
 * tests if the garbage inputs into the four 
 * CP2 functions of rtc would return -1
 * Inputs: None
 * Outputs: 1 on success
 * Side Effects: None
 * Coverage: tc_open, rtc_close, rtc_read, rtc_write
 * Files: none
 */
int rtc_garbage_input_test() {
	TEST_HEADER;
	int32_t fd = 1;
	int bad_hertz = 7; //a random # NOT of 2's power
	int good_hertz = 16; //a random # of 2's power
	int* buf = &good_hertz;
	int32_t nbytes = 4;

	if ((rtc_write(fd, &bad_hertz, nbytes) == -1)
		|| (rtc_write(fd, NULL, nbytes) == -1) 		//test buf on write
		|| (rtc_read(fd, NULL, nbytes) == -1)) { 	//test buf on read
		return 1; //pass				   
	} else {
		return 0; //fail
	}
	
	fd = 8; //a random # NOT of [0, 7]
	if ((rtc_write(fd, buf, nbytes) == -1) 			//test fd on write
		|| (rtc_close(fd) == -1)					//test fd on close
		|| (rtc_read(fd, buf, nbytes) == -1)) { 	//test fd on read		
		return 1; //pass
	} else {
		return 0; //fail
	}

	nbytes = 3; //a random # NOT equal to 4
	fd = 7; //a random # of [0, 7]
	if ((rtc_write(fd, buf, nbytes) == -1)			//test nbytes on write
		|| (rtc_read(fd, buf, nbytes) == -1)) { 	//test nbytes on read
		return 1; //pass
	} else {
		return 0; //fail
	}

	uint8_t* filename = NULL;
	if (rtc_open(filename) == -1) {			//test filename on open
		return 1; //pass
	} else {
		return 0; //fail
	}
}

/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	clear();
	// TEST_OUTPUT("read_file_out_of_length", read_file_out_of_length());
	// TEST_OUTPUT("partial_read_file", partial_read_file());
 	// TEST_OUTPUT("file_open_test", file_open_test());
	// TEST_OUTPUT("file_write_test", file_write_test());
 	// TEST_OUTPUT("file_close_test", file_close_test());
 	//TEST_OUTPUT("dir_test", dir_test());

	//TEST_OUTPUT("file_test_frame0", file_test("frame0.txt", 187));
	// TEST_OUTPUT("file_test_frame1", file_test("frame1.txt", 174));
	// TEST_OUTPUT("file_test_ls", file_test("ls", 5349));
	//TEST_OUTPUT("file_test_grep", file_test("grep", 6149));
	//TEST_OUTPUT("file_test_shell", file_test("shell", 5605));
	// TEST_OUTPUT("file_test_verylong", file_test("verylargetextwithverylongname.txt", 5277));

	// TEST_OUTPUT("page_table_value_test", page_table_value_test());
	// TEST_OUTPUT("page_table_value_test", page_table_value_test());
	//TEST_OUTPUT("page_fault_test", page_fault_test());

	//div_by_zero_test();
	
	//TEST_OUTPUT("rtc_interrupt_test", rtc_interrupt_test());
	//TEST_OUTPUT("keyboard_scancode_test", keyboard_scancode_test());
	//TEST_OUTPUT("garbage_input_test", garbage_input_test());
	//TEST_OUTPUT("return_value_test", return_value_test());
	
	//keyboard_terminal_interactive_test(kbuf_size);
	// keyboard_terminal_interactive_test(50);
	// TEST_OUTPUT("terminal write null test", keyboard_terminal_null_test());

	// TEST_OUTPUT("rtc_open_test", rtc_open_test());
	//TEST_OUTPUT("rtc_read_test", rtc_read_test());
	// TEST_OUTPUT("rtc_write_test", rtc_write_test());
	//TEST_OUTPUT("rtc_close_test", rtc_close_test());
	//TEST_OUTPUT("rtc_garbage_input_test", rtc_garbage_input_test());
	//TEST_OUTPUT("rtc_set_rate_test", rtc_set_rate_test());
}
