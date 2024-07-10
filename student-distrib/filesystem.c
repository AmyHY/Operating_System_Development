#include "filesystem.h"

/* 
 * fileSystem_init
 *   DESCRIPTION: get necessary setup information for the file system and store in the info struct.
 *   INPUTS: starting address of page directory.
 *   OUTPUTS: return 0 if init successfully, return -1 for invalid input.
 */
int32_t fileSystem_init(uint32_t * fs_start){
    //check for invalid pointers
    if(fs_start == NULL){
        return -1;
    }
    
    info.filesys_start = fs_start;
    // Set up the starting address of the boot block.
    info.boot_block_ptr = (boot_block_t *) fs_start;  
    // Store the total number of index nodes.
    info.total_inode = (info.boot_block_ptr)->inode_count;
    // Store the total number of data blocks.
    info.total_data = (info.boot_block_ptr)->data_count;

    info.inode_start = (inode_t * )(info.boot_block_ptr + 1);

    info.data_blocks_start = (data_block_t *) (info.inode_start + info.total_inode);

    return 0;
}

/* 
 * read_dentry_by_name
 *   DESCRIPTION: find dentry with filename equals to the fname provided, and store found dentry in the second parameter.
 *   INPUTS: the filename to search for, and dentry buffer.
 *   OUTPUTS: return 0 if found match, return -1 if not found or invalid input.
 */
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry){
    // Check for invalid file name.
    if (fname == NULL) {
        return -1;
    }

    // Get size of fname
    uint32_t fname_size;
    fname_size = strlen((int8_t*)fname);
    
    //check if the length of the fname is valid
    if (fname_size == 0) {
        return -1;
    }
    // If more than 32 bytes are read, then the length should be 32.
    if(fname_size > MAX_FILENAME_LEN){
        return -1;
    }
    int i;
    // Search for file with fname
    for(i = 0; i < (info.boot_block_ptr->dir_count); i++){
        dentry_t curr_dentry = (info.boot_block_ptr)->direntries[i];
        uint32_t curr_fnamesize;
        curr_fnamesize = strlen((int8_t*)curr_dentry.filename);
        if (curr_fnamesize > MAX_FILENAME_LEN) {
            curr_fnamesize = MAX_FILENAME_LEN;
        }
        // If file is found call read_dentry_by_index and return 0
        if((curr_fnamesize == fname_size) && (strncmp((int8_t*)fname, curr_dentry.filename, curr_fnamesize) == 0)){
            read_dentry_by_index(i, dentry);
            return 0;
        }
    }

    // File not found
    return -1;

}

/* 
 * read_dentry_by_index
 *   DESCRIPTION: find dentry in the given index, and store found dentry in the second parameter.
 *   INPUTS: the index to search for, and dentry buffer.
 *   OUTPUTS: return 0 if found match, return -1 if not found or invalid input.
 */
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry){

    // Check if index is valid
    if(index >= (info.boot_block_ptr)->dir_count || index < 0) {
        return -1;
    }

    // Copy dentry at index to arg passed in
    memcpy((void *)dentry, &(info.boot_block_ptr->direntries[index]), DENTRY_SIZE);
    return 0;
}


/* 
 * read_data
 *   DESCRIPTION: In the given inode, read length bytes from the file starting from offset, and store results in buf.
 *   INPUTS: the index node number, the starting offset, storing buffer, and the length to read.
 *   OUTPUTS: return 0 if read until the end of the file, return a positive number if read length bytes, return -1 for invalid inode number or data block index.
 */
int32_t read_data (uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length){
    // Check inode is within range
    if(inode < 0 || inode >= info.boot_block_ptr->inode_count){
        return -1;
    }

    inode_t* inode_ptr = info.inode_start + inode;
    // Get the file size from the corresponding index node.
    uint32_t file_size = inode_ptr->length;
    //if starting position is at end of file or beyond end of file, return 0
    if(offset >= file_size){
        return 0;
    }
    //if length is greater than what we can read (ie until the end of file), change length
    if((file_size - offset) < length){
        length = file_size - offset;
    }
    // Record the number of bytes read.
    int32_t  bytes_read = 0;
    
    while (length != 0) {
        // Get the index of the current data block to read.
        int32_t curr_data_block_idx = inode_ptr->data_block_idx[offset / DATA_BLOCK_SIZE];
        // If the inode contains a data block that is out of range.
        if (curr_data_block_idx >= info.boot_block_ptr->data_count) {
            return -1;
        }
        // Get the offset to read inside the data block.
        uint32_t data_block_offset = offset % DATA_BLOCK_SIZE;
        // Initialize to the remaining bytes in the block.
        uint32_t bytes_to_read = DATA_BLOCK_SIZE - data_block_offset;
        // If the file doesn't take over the entire block, we only read the valid bytes.
        if (file_size - offset < bytes_to_read) {
            bytes_to_read = file_size - offset;
        }
        // If we want to read fewer bytes than the remaining bytes in the current block.
        if (length < bytes_to_read) {
            bytes_to_read = length;
        }
        data_block_t* data_block_to_read = info.data_blocks_start + curr_data_block_idx;
        // Copy bytes from the data block to the buffer.
        memcpy(buf+bytes_read, &(data_block_to_read->data[data_block_offset]), bytes_to_read);
        bytes_read += bytes_to_read;
        length -= bytes_to_read;
        offset += bytes_to_read;
    }
   
    return bytes_read;
    
}

/* 
 * dir_open
 *   DESCRIPTION: open a directory for future operations.
 *   INPUTS: the directory name.
 *   OUTPUTS: the file descriptor (not implemented yet). Currently returning whether we can find the file with 0 as success, -1 as failure.
 */
int32_t dir_open(const uint8_t* filename){
    // Indicating that we will start to read file at index 0 of the dentry list. 
    info.counter = 0;
    return read_dentry_by_name(filename, &(info.dentry));
}

/* 
 * dir_read
 *   DESCRIPTION: read the name of a file in the directory.
 *   INPUTS: the file descriptor, the storing buffer, the bytes to read (not used).
 *   OUTPUTS: return a positive number as bytes read if successfully read a file name. return 0 if we have read all files in the directory. return -1 for invalid file descriptor.
 */
int32_t dir_read(int32_t fd, void * buf, int32_t nbytes){
    // If the file descriptor is out of range.
    if(fd < 0 || fd >= 8){
        return -1;
    }
    // printf("current counter: %d\n", info.counter);
    // If we have read all files in the directory.
    if (read_dentry_by_index(info.counter, &(info.dentry)) == -1) {
        return 0;
    }
    info.counter += 1;
    strncpy(buf, info.dentry.filename, MAX_FILENAME_LEN);
    // Get the size of the file name and return.
    int len = strlen(info.dentry.filename);
    if (len >= MAX_FILENAME_LEN) {
        return MAX_FILENAME_LEN;
    }
    return len;
}

/* 
 * dir_write
 *   DESCRIPTION: do nothing since the file system is read-only.
 *   INPUTS: the file descriptor, the storing buffer, the bytes to read (not used).
 *   OUTPUTS: always return -1 for failure.
 */
int32_t dir_write(int32_t fd, const void * buf, int32_t nbytes){
    return -1;
}

/* 
 * dir_close
 *   DESCRIPTION: close the file descriptor (not implemented yet).
 *   INPUTS: the file descriptor, the storing buffer, the bytes to read (not used).
 *   OUTPUTS: return -1 for invalid file descriptor. 
 */
int32_t dir_close(int32_t fd){
    // The user should not close file descriptor 0 (stdin) and 1 (stdout)
    if(fd < 2 || fd >= 8){
        return -1;
    }
    return 0;
}



/* 
 * dir_open
 *   DESCRIPTION: open a file for future operations.
 *   INPUTS: the file name.
 *   OUTPUTS: the file descriptor (not implemented yet). Currently returning whether we can find the file with 0 as success, -1 as failure.
 */
int32_t file_open(const uint8_t* filename){
    // Return whether we can find the dentry for the file. Also, store the found dentry in the info struct for future reference.
    return read_dentry_by_name(filename, &(info.dentry));
}

/* 
 * file_read
 *   DESCRIPTION: read nbytes from the file if applicable
 *   INPUTS: the file descriptor, the storing buffer, the bytes to read.
 *   OUTPUTS: return 0 if read until the end of the file, return a positive number if read nbyte bytes, return -1 for invalid file descriptor.
 */
int32_t file_read(int32_t fd, void * buf, int32_t nbytes){
    // If the file descriptor is out of range.
    if(fd < 0 || fd >= 8){
        return -1;
    }
    pcb_t* curr_pcb = get_pcb(schedule[active_term_idx]);
    file_descriptor_t* fd_array = curr_pcb->fd_array;
    // Get the inode number from the dentry read from file_open()
    uint32_t num_bytes_read = read_data(fd_array[fd].inode, fd_array[fd].file_pos, buf, nbytes);
    if (num_bytes_read != -1) {
        fd_array[fd].file_pos += num_bytes_read;
    }
    return num_bytes_read;
}

/* 
 * file_write
 *   DESCRIPTION: do nothing since the file system is read-only.
 *   INPUTS: the file descriptor, the storing buffer, the bytes to read (not used).
 *   OUTPUTS: always return -1 for failure.
 */
int32_t file_write(int32_t fd, const void * buf, int32_t nbytes){
    return -1;
}

/* 
 * file_close
 *   DESCRIPTION: close the file descriptor (not implemented yet).
 *   INPUTS: the file descriptor, the storing buffer, the bytes to read (not used).
 *   OUTPUTS: return -1 for invalid file descriptor. 
 */
int32_t file_close(int32_t fd){
    // The user should not close file descriptor 0 (stdin) and 1 (stdout)
    if(fd < 2 || fd >= 8){
        return -1;
    }
    return 0;
}
