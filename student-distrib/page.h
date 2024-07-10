#ifndef PAGE_H
#define PAGE_H

// Number of entries in a page table.
#define NUM_ENTRIES 1024
// Size of an entry in a page table in bytes
#define ENTRY_SIZE 4
// Page table size in bytes
#define TABLE_SIZE (NUM_ENTRIES*ENTRY_SIZE)
// Size of a 4KB page table in bytes.
#define PAGE_SIZE_4KB 4096
// A page table is 4KB, which is 4096 = 2 ** 12.
#define PAGE_TABLE_NUM_OFFSET 12
// 4MB (page size) / 1B (unit for each entry) = 2 ** 22.
#define MB_PAGE_NUM_OFFSET 22
// 4KB (page size) / 1B (unit for each entry) = 2 ** 12.
#define KB_PAGE_NUM_OFFSET 12
// Physical address of user memory.
#define USER_MEMORY_VIR 0x8000000
// Start and end of the video memory in physical memory.
#define VIDEO_MEMORY_START 0xB8000
#define VIDEO_MEMORY_END 0xBFFFF

//Video page for terminals
#define VID_PAGE_T0     0xBA000
#define VID_PAGE_T1     0xBB000
#define VID_PAGE_T2     0xBC000

// Physical address of kernel memory.
#define KERNEL_MEMORY 0x400000
//virtual addr of the 4kb page used for vidmap
#define VID_MAP_VIR 0x8800000

// All attributes in page directory entry for 4KB page tables as defined in the manual.
typedef struct __attribute__((packed)) page_dir_entry_kb {
    int present                      : 1;
    int read_write                   : 1;
    int user_supervisor              : 1;
    int write_through                : 1;
    int cache_disabled               : 1;
    int accessed                     : 1;
    int reserved                     : 1;
    int page_size                    : 1;
    int global_page                  : 1;
    int available                    : 3;
    int base_address                 : 20;
} page_dir_entry_kb;

// All attributes in page directory entry for 4MB page as defined in the manual.
typedef struct __attribute__((packed)) page_dir_entry_mb {
    int present                      : 1;
    int read_write                   : 1;
    int user_supervisor              : 1;
    int write_through                : 1;
    int cache_disabled               : 1;
    int accessed                     : 1;
    int dirty                        : 1;
    int page_size                    : 1;
    int global_page                  : 1;
    int available                    : 3;
    int page_table_attribute_index   : 1;
    int reserved                     : 9;
    int base_address                 : 10;
} page_dir_entry_mb;

// All attributes in page table entry for 4KB page as defined in the manual.
typedef struct __attribute__((packed)) page_table_entry {
    int present                      : 1;
    int read_write                   : 1;
    int user_supervisor              : 1;
    int write_through                : 1;
    int cache_disabled               : 1;
    int accessed                     : 1;
    int dirty                        : 1;
    int page_table_attribute_index   : 1;
    int global_page                  : 1;
    int available                    : 3;
    int base_address                 : 20;
} page_table_entry;

// array of page directory entry as the page directory, each with 4 bytes.
int page_directory[NUM_ENTRIES] __attribute__((aligned (TABLE_SIZE)));
// array of page table entry as the page table for the first 4MB of virtual memory, each with 4 bytes.
page_table_entry page_table[NUM_ENTRIES] __attribute__((aligned (TABLE_SIZE)));
page_table_entry vid_map_page_table[NUM_ENTRIES] __attribute__((aligned (TABLE_SIZE)));

// Set up each attributes for page directory entries for 4KB page tables.
void setup_page_dir_entry_kb(page_dir_entry_kb* entry, int present_, int read_write_, 
    int user_supervisor_, int write_through_, int cache_disabled_, int accessed_, 
    int reserved_, int page_size_, int global_page_, int available_, int base_address_);

// Set up each attributes for page directory entries for 4MB pages.
void setup_page_dir_entry_mb(page_dir_entry_mb* entry, int present_, int read_write_, 
    int user_supervisor_, int write_through_, int cache_disabled_, int accessed_, int dirty_, 
    int page_size_, int global_page_, int available_, int pta_, int reserved, int base_address_);

// Set up each attributes for page tables entries for 4KB page.
void setup_page_table_entry(page_table_entry* entry, int present_, int read_write_, 
    int user_supervisor_, int write_through_, int cache_disabled_, int accessed_, 
    int dirty_, int pta_, int global_page_, int available_, int base_address_);

// Initial Page tables and page directories.
void setup_paging();

#endif

