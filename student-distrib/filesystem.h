#ifndef _FILESYSTEM_H
#define _FILESYSTEM_H

#include "lib.h"
#include "syscall.h"
#include "terminal.h"

#define BLOCK_TOTAL_BYTES       4096        // 4KB per block, 4096 bytes per block
#define DENTRY_SIZE             64          // Size of a de.ntry in bytes.
#define MAX_FILENAME_LEN        32          // Maximum length of a file in bytes
#define BOOT_RESERVED_NUM       52          // Number of reserved bytes in the boot block.
#define MAX_DENTRY_NUM          63          // (4KB/64B) - 1 (stats block) = 63 possible dentries
#define DENTRY_RESERVED_NUM     24          // Number of reserved bytes in the the dentry.
#define DATA_BLOCK_SIZE         4096        // 4KB datablocks
#define NUM_DATA_BLOCK_IN_INODE 1023        // (4096 (block size) / 4 (entry size)) - 1 (length)
#define FILE_TYPE_RTC           0           // File type number for RTC files
#define FILE_TYPE_DIR           1           // File type number for directory files
#define FILE_TYPE_REG           2           // File type number for regular files

// Represent a dentry containing the file name, file type (0, 1, 2), and the index node number.
typedef struct dentry{
    int8_t filename[MAX_FILENAME_LEN];
    int32_t filetype;
    int32_t inode_num;
    int8_t reserved[DENTRY_RESERVED_NUM];
} dentry_t;

// Represent the structure in the boot block.
typedef struct boot_block{
    int32_t  dir_count;
    int32_t  inode_count;
    int32_t  data_count;
    int8_t   reserved[BOOT_RESERVED_NUM];
    dentry_t direntries[MAX_DENTRY_NUM];
} boot_block_t;

// Represent an index node.
typedef struct inode{
    int32_t length;
    int32_t data_block_idx[NUM_DATA_BLOCK_IN_INODE];
} inode_t;

// Represent a data block.
typedef struct data_block{
    char data[DATA_BLOCK_SIZE]; 
} data_block_t;

//struct holding all the global variables for the file system.
typedef struct file_sys_info {
    uint32_t*      filesys_start;       // starting addr of the file system.     
    boot_block_t*  boot_block_ptr;      // starting addr of the boot block.
    inode_t*       inode_start;         // starting addr of the index nodes  
    data_block_t*  data_blocks_start;   // starting addr of the data blocks.  
    int32_t        total_data;          // total number of data block.
    int32_t        total_inode;         // total number of inodes.
    int32_t        counter;             // Counter to record which file to read next for dir_read(). 
    dentry_t       dentry;              // Keep track of which file has been openned by file_open().
} file_sys_info;

// Store necessary information for the file system.
file_sys_info info;

// stores dentry for file to be run in execute
dentry_t file_dentry;

// Initialize global variables for the file system.
int32_t fileSystem_init(uint32_t * fs_start);

// Find the dentry with the filename as the one given.
int32_t read_dentry_by_name (const uint8_t* fname, dentry_t* dentry);
// Find the dentry at the given index.
int32_t read_dentry_by_index (uint32_t index, dentry_t* dentry);
// Read length bytes of data from the inode starting with offset, and stored results in buf.
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

// Open a directory.
int32_t dir_open(const uint8_t* filename);
// Read a filename in the directory.
int32_t dir_read(int32_t fd, void * buf, int32_t nbytes);
// Write to the directory.
int32_t dir_write(int32_t fd, const void * buf, int32_t nbytes);
// Close the directory.
int32_t dir_close(int32_t fd);

// Open a file to read.
int32_t file_open(const uint8_t* filename);
// Read nbytes from a file and store results in the buf.
int32_t file_read(int32_t fd, void * buf, int32_t nbytes);
// Write to the file.
int32_t file_write(int32_t fd, const void * buf, int32_t nbytes);
// Close the file.
int32_t file_close(int32_t fd);



#endif
