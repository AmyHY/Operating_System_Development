#ifndef SYSCALL_H
#define SYSCALL_H

#include "types.h"
#include "lib.h"
#include "filesystem.h"
#include "page.h"
#include "terminal.h"
#include "rtc.h"
#include "x86_desc.h"
#include "multiboot.h"

#define MAX_PID                  6           // Maximum number of allowed process. 
#define PAGE_SIZE_4MB            0x400000    // 4mb
#define USER_MEM_START_PHY       0x800000    // 8MB
#define KERNEL_MEM_START         0x800000    // 8MB
#define PROCESS_STACK_SIZE       0x2000      // 8kb
#define MAX_FILES                8           // Maximum number of files allowed to open simultaneously.
#define USER_MEM_START_VIR       0x8000000  // The starting virtual address of the block for the user program memory (first 10 bits for 0x08048000)
#define PROGRAM_START            0x08048000 // The start of the program code in virtual memory
#define EIP_START_BYTE           24         // Index of the bytes storing the user program start
#define FD_UNUSED                0          // File type number for unused file descriptors
#define FD_USED                  1          // File type number for file descriptors in use
#define ARGS_BUF_SIZE            1024       // large enough number to store args
#define NUM_TERMS           3           //support max of 3 terminals

//array that keep track of availablity of pids (0 -available, 1 - unavailable)
uint32_t pid_status[MAX_PID]; //supports max of two processes

//struct of function ptrs to open,read,write,close ops.
typedef struct file_op_table{
   int32_t (*open) (const uint8_t* filename);
   int32_t (*read) (int32_t fd, void* buf, int32_t nbytes);
   int32_t (*write) (int32_t fd, const void* buf, int32_t nbytes);
   int32_t (*close) (int32_t fd);
} file_op_table_t;

typedef struct file_descriptor{
    file_op_table_t* file_op_table_ptr;
    int32_t  inode;
    int32_t  file_pos;
    int8_t   flags;
} file_descriptor_t;

// Process Control Block. Used to stored information for context switch in execute() and halt()
typedef struct pcb{
    uint32_t pid;
    uint32_t parent_pid;
    uint32_t user_eip;          //used to restart shell for base shells
    uint32_t kernel_ebp; 
    uint32_t exe_ebp;
    uint32_t kernel_esp;
    uint32_t exe_esp;
    //rtc 
    uint32_t max_rtc_count;
    uint32_t rtc_interrupt;
    uint32_t rtc_fd_idx;
    file_descriptor_t fd_array[MAX_FILES];
    uint8_t args[ARGS_BUF_SIZE]; //args parsed from the cmd in execute; used for getargs
} pcb_t;

//array that holds the current active process pid in each terminal
uint32_t schedule[NUM_TERMS];

// Operator tables for each file type
file_op_table_t stdin_op;
file_op_table_t stdout_op;
file_op_table_t dir_op;
file_op_table_t file_op;
file_op_table_t rtc_op;
file_op_table_t terminal_op;

//checking if file is executable
int32_t file_executable(const uint8_t* filename);
//setting up file_op_tables
void setup_file_op_table();
//set up process memory
void setup_process_memory(uint32_t pid);
//switches the current active process
void process_switch(uint32_t from_pid, uint32_t to_pid);
//stdin write and stdout read function (return error)
int32_t stdin_write(int32_t fd, const void* buf, int32_t nbytes);
int32_t stdout_read(int32_t fd, void* buf, int32_t nbytes);
pcb_t* get_pcb(uint32_t pid);

// Quit a program after execution.
int32_t halt(uint8_t status);
// Execute a user level program.
int32_t execute(const uint8_t* command);
// Read a file by file descriptor.
int32_t read(int32_t fd, void* buf, int32_t nbytes);
// Write to a file by file descriptor.
int32_t write(int32_t fd, const void* buf, int32_t nbytes);
// Open a file by file name.
int32_t open(const uint8_t* filename);
// Close a file descriptor if valid.
int32_t close(int32_t fd);
// Checks for valid inputs and copy args into buf
int32_t getargs(uint8_t* buf, int32_t nbytes);
// Map virtual memory to the video memory.
int32_t vidmap(uint8_t** screen_start);
int32_t set_handler(int32_t signum, void* handler_address);
int32_t sigreturn();

#endif
