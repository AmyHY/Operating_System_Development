#include "page.h"


/* 
 * setup_cr
 *   DESCRIPTION: set up values for CR0, CR3, CR4 for usage of paging.
 *   INPUTS: base address of page directory.
 *   OUTPUTS: none
 */
extern void setup_cr(int page_directory);

/* 
 * setup_page_dir_entry_kb
 *   DESCRIPTION: set up value for each attribute in page directory entry for 4KB page tables
 *   INPUTS: values for each attributes.
 *   OUTPUTS: none
 */
void setup_page_dir_entry_kb(page_dir_entry_kb* entry, int present_, int read_write_, 
    int user_supervisor_, int write_through_, int cache_disabled_, int accessed_, 
    int reserved_, int page_size_, int global_page_, int available_, int base_address_) {
        entry->present = present_;
        entry->read_write = read_write_;
        entry->user_supervisor = user_supervisor_;
        entry->write_through = write_through_;
        entry->cache_disabled = cache_disabled_;
        entry->accessed = accessed_;
        entry->reserved = reserved_;
        entry->page_size = page_size_;
        entry->global_page = global_page_;
        entry->available = available_;
        entry->base_address = base_address_;
    }

/* 
 * setup_page_dir_entry_mb
 *   DESCRIPTION: set up value for each attribute in page directory entry for 4MB page
 *   INPUTS: values for each attributes.
 *   OUTPUTS: none
 */
void setup_page_dir_entry_mb(page_dir_entry_mb* entry, int present_, int read_write_, 
    int user_supervisor_, int write_through_, int cache_disabled_, int accessed_, int dirty_, 
    int page_size_, int global_page_, int available_, int pta_, int reserved, int base_address_) {
        entry->present = present_;
        entry->read_write = read_write_;
        entry->user_supervisor = user_supervisor_;
        entry->write_through = write_through_;
        entry->cache_disabled = cache_disabled_;
        entry->accessed = accessed_;
        entry->dirty = dirty_;
        entry->page_size = page_size_;
        entry->global_page = global_page_;
        entry->available = available_;
        entry->page_table_attribute_index = pta_;
        entry->reserved = reserved;
        entry->base_address = base_address_;
    }

/* 
 * setup_page_table_entry
 *   DESCRIPTION: set up value for each attribute in page table entry for 4KB page
 *   INPUTS: values for each attributes.
 *   OUTPUTS: none
 */
void setup_page_table_entry(page_table_entry* entry, int present_, int read_write_, 
    int user_supervisor_, int write_through_, int cache_disabled_, int accessed_, 
    int dirty_, int pta_, int global_page_, int available_, int base_address_) {
        entry->present = present_;
        entry->read_write = read_write_;
        entry->user_supervisor = user_supervisor_;
        entry->write_through = write_through_;
        entry->cache_disabled = cache_disabled_;
        entry->accessed = accessed_;
        entry->dirty = dirty_;
        entry->page_table_attribute_index = pta_;
        entry->global_page = global_page_;
        entry->available = available_;
        entry->base_address = base_address_;
    }

/* 
 * setup_paging
 *   DESCRIPTION: initialize page table and page directory.
 *   INPUTS: none
 *   OUTPUTS: none
 */
void setup_paging() {
    // Set up page directory entries for the first 4MB.
    // Explanation of each attribute set in the first entry:
    // present: 1 -> the page table is setup in the physical memory
    // read_write: 1 -> the page table should be both read and writeable
    // user_supervisor: 0 -> we need to protect the video memory in the supervisor mode
    // write_through: 0 -> use write-back policy: write only to the cache, and update main memory after several write operations.
    // cache_disabled: 0 -> enable caching for performance
    // accessed: 0 -> the flag should be set the first time the page_table is accessed.
    // reserved: 0 -> If the flag is not 0, a page fault would be generated.
    // page_size: 0 -> it is pointed to a 4KB page table.
    // global_page: 0 -> the attribute should be ignored in page directory entries.
    // base_address -> get the most significant 22 bits since the page table has 1024 entries and has 10 bit offsets.
    page_dir_entry_kb first_dir_entry;
    setup_page_dir_entry_kb(&first_dir_entry, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, (int)page_table>>PAGE_TABLE_NUM_OFFSET);
    page_directory[0] = *(int*)&first_dir_entry;
    int i;
    // Set up page table entries for the first 4MB.
    for (i = 0; i < NUM_ENTRIES; i++) {
        // present -> Video memory is in the physical memory while the others are not present
        // read_write: 1 -> the video memory should be both read and writable
        // user_supervisor: 0 -> we need to protect the video memory in the supervisor mode
        // write_through: 0 -> use write-back policy: write only to the cache, and update main memory after several write operations.
        // cache_disabled: 0 -> enable caching for performance
        // accessed: 0 -> the flag should be set the first time the page_table is accessed.
        // dirty: 0 -> the flag should be set the first time the page is modified.
        // page_table_attribute_index: 0 -> disable PAT for safe.
        // global_page: 0 -> not a global page
        // base_address -> If the page is for the video memory, then set the current position in virtual memory.
        // since the video memory should be in the same position in both virtual and physical memory.
        page_table_entry table_entry;
        // The purpose is the map the virtual address 0xB8000 to the physical address at 0xB8000 (video memory)
        if (i == (VIDEO_MEMORY_START >> KB_PAGE_NUM_OFFSET)) {
            setup_page_table_entry(&table_entry, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, VIDEO_MEMORY_START >> KB_PAGE_NUM_OFFSET);
        } 
        else if (i == (VID_PAGE_T0 >> KB_PAGE_NUM_OFFSET)) {
            setup_page_table_entry(&table_entry, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, VID_PAGE_T0 >> KB_PAGE_NUM_OFFSET);
        } 
        else if (i == (VID_PAGE_T1 >> KB_PAGE_NUM_OFFSET)) {
            setup_page_table_entry(&table_entry, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, VID_PAGE_T1 >> KB_PAGE_NUM_OFFSET);
        } 
        else if (i == (VID_PAGE_T2 >> KB_PAGE_NUM_OFFSET)) {
            setup_page_table_entry(&table_entry, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, VID_PAGE_T2 >> KB_PAGE_NUM_OFFSET);
        } 
        else {
            setup_page_table_entry(&table_entry, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0);
        }
        page_table[i] = table_entry;
    }
    // Set up the remaining page directory entries.
    for (i = 1; i < NUM_ENTRIES; i++) {
        page_dir_entry_mb page_dir_entry;
        // present -> except for the kernel code, all other pages are not present in physical memory.
        // read_write: 1 -> kernel code, user code image and other pages should all be set as writable (TODO: why kernel code should be writable).
        // user_supervisor -> kernel code should be protected in supervisor mode, the others are fine.
        // write_through: 0 -> use write-back policy: write only to the cache, and update main memory after several write operations.
        // cache_disabled: 0 -> enable caching for performance
        // accessed: 0 -> the flag should be set the first time the page_table is accessed.
        // dirty: 0 -> the flag should be set the first time the page is modified.
        // page_size: 1 -> it is pointed to a 4MB page.
        // global_page: 0 -> not a global page
        // page_table_attribute_index: 0 -> disable PAT for safe.
        // reserved: 0 -> set as default.
        // base_address -> If the page is for the video memory, then set the current position in virtual memory.
        // since the video memory should be in the same position in both virtual and physical memory.
        if (i == 1) {
            // Set up the page directory entry for the kernel code.
            // The purpose is the map the virtual address 0x400000 to the physical address at 0x400000 (kernel memory). Both virtual and physical memory should span for 4MB
            setup_page_dir_entry_mb(&page_dir_entry, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, KERNEL_MEMORY>>MB_PAGE_NUM_OFFSET);
        } else {
            setup_page_dir_entry_mb(&page_dir_entry, 0, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0);
        }
        page_directory[i] = *(int*)&page_dir_entry;
    }
    setup_cr((int) page_directory);
}


