#include "syscall.h"

/* 
 * halt
 *   DESCRIPTION: terminates current process and return back to the parent process; 
 *                restart shell if current process is the root shell
 *   INPUTS: status (returned to the parent process)
 *   OUTPUTS: 0 on success
 */
int32_t halt(uint8_t status) {
    
    uint32_t curr_pid = schedule[active_term_idx];
    pcb_t* curr_pcb = get_pcb(curr_pid);
    //restart if attempting to halt of root shell of any terminal
    if(curr_pcb->parent_pid == -1){
        cli();
        uint32_t prog_eip;
        prog_eip = curr_pcb->user_eip;
        uint32_t prog_esp;
        prog_esp = USER_MEM_START_VIR + PAGE_SIZE_4MB - 4;

        sti();
        asm volatile(
                    "pushl %3           \n\t"   // push ds onto stack          
                    "pushl %1           \n\t"   // push esp onto stack
                    "pushfl             \n\t"   // push all flags onto stack
                    "pushl %2           \n\t"   // push cs onto stack       
                    "pushl %0           \n\t"   // push eip onto stack     
                    "iret               \n\t"   
                    :
                    : "r"(prog_eip), "r"(prog_esp), "r" (USER_CS), "r" (USER_DS)
                    : "memory"
                 );

    }

    else{
        cli();
        //close current process
        pid_status[curr_pid] = 0;
        
        // get parent pid
        uint32_t parent_pid = curr_pcb->parent_pid;
        schedule[active_term_idx] = parent_pid; //update schedule pid

        // Restore paging for the parent process.
        setup_process_memory(parent_pid);

        // Set all file descriptor to be not used.
        int i; //for-loop index
        for (i = 0; i < MAX_FILES; i++) {  
            if(curr_pcb->fd_array[i].flags == 1){
                close(i);
            }   
        }
        // Set esp0 to be the start of the kernel memory for the process.
        tss.esp0 = KERNEL_MEM_START - (parent_pid*PROCESS_STACK_SIZE) - 4; 
        tss.ss0 = KERNEL_DS;

        //get kernel_ebp to return back to execute program
        uint32_t ebp;
        ebp = curr_pcb->exe_ebp;
        uint32_t esp;
        esp = curr_pcb->exe_esp;

        sti();
        //asm volatile ("movl %0, %%eax\n" : :"r"((int)status));
        // Return back to the execute program of the child process. We saved the ebp for the execute() program. 
        // So using the leave & ret command, we can return from the execute() and back to the parent process (next instruction after system call).
        asm volatile(   "movl %0, %%ebp             \n\t"   // restore ebp for the parent program.
                        "movl %1, %%esp             \n\t"   // restore ebp for the parent program.
                        "movl %2, %%eax              \n\t"   // restore ebp for the parent program.
                        "leave                      \n\t"   // %esp = %ebp, popl %ebp
                        "ret                        \n\t"   // movl (%esp) %eip, %esp = %esp + 4
                        :
                        : "r"(ebp),"r"(esp), "r" ((uint32_t)status)
                        : "eax", "ebp", "esp"
                        );
        
    }

    return 0;
}

/* 
 * set_process_memory
 *   DESCRIPTION: helper function to map user program phys addr to user program virtual (128MB)
 *   INPUTS: pid of the program to be mapped
 *   OUTPUTS: none
 */

void setup_process_memory(uint32_t pid) {
    // Set up page directory to map virtual address 0x08048000 to physical address 8MB (0x800000) + pid * 4MB (0x400000)
    uint32_t paddr = USER_MEM_START_PHY + (pid * PAGE_SIZE_4MB);
    page_dir_entry_mb prog_dir_entry; 
    setup_page_dir_entry_mb(&prog_dir_entry,1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, paddr>>MB_PAGE_NUM_OFFSET);
    page_directory[USER_MEMORY_VIR >> MB_PAGE_NUM_OFFSET] = *(int*)&prog_dir_entry;
    //flush_tlb: processor will flush the tlb once it notices that cr3 has been changed
    asm volatile ("movl %cr3, %eax  \n\t"
                  "movl %eax, %cr3  \n\t"      
                );
}

/* 
 * execute
 *   DESCRIPTION: load and execute a new program
 *   INPUTS: command
 *   OUTPUTS: 0 on success, -1 on failure
 */
int32_t execute(const uint8_t* command) {
    cli();
    //parse cmd
    uint32_t i;
    uint32_t exe_end;
    exe_end = 0;                         
    if (command == NULL) {
        return -1;
    }

    uint8_t filename[MAX_FILENAME_LEN];
    uint8_t args_temp[ARGS_BUF_SIZE];
    //clear filename & arg to '\0'
    for(i = 0; i < MAX_FILENAME_LEN; i++){
        filename[i] = '\0';
    }

    for(i = 0; i < ARGS_BUF_SIZE; i++){
        args_temp[i] = '\0';
    }

    //parse through cmd to get the filename/////////////////////////////////////////////
    for (i = 0; i < strlen((const int8_t*)command); i++) {
        if (command[i] != ' ') {
            filename[i] = (uint8_t) command[i];
            exe_end= i;
        } else {
            break;
        }
    }

    //parse through rest of cmd to get args/////////////////////////////////////////////
    uint32_t args_idx;
    uint32_t args_start_idx;
    args_idx = 0;
    args_start_idx = exe_end + 2; //start parse 1 space after exe_end (i = exe_end + 2)

    for(i = args_start_idx; i < strlen((const int8_t*)command); i++){   
      
        args_temp[args_idx] = (uint8_t) command[i];
        args_idx++;
    }

    
    //check file valid///////////////////////////////////////////////////////////////////
    //check if file exist, if it does store the dentry for file 
    if(read_dentry_by_name(filename, &file_dentry) == -1){
        return -1;
    }

    if (file_executable(filename) == -1) {
        return -1;
    }

    //paging (mapping v mem to p mem based on pid)/////////////////////////////////////////
    uint32_t new_pid = MAX_PID;

    //check which pid is available
    for(i =0; i < MAX_PID; i++){
        if(pid_status[i] == 0){
            new_pid = i;
            pid_status[i] = 1;
            break;
        }
    }

    //if pid_num is not set -> no pid available
    if(new_pid == MAX_PID){
        return -1;
    }
    setup_process_memory(new_pid);

    // Copy the user code to the corresponding physical memory address (8MB (0x800000) + pid * 4MB (0x400000) + 0x48000)
    inode_t* file_inode = info.inode_start + file_dentry.inode_num;
    read_data(file_dentry.inode_num,0,(uint8_t *)PROGRAM_START,file_inode->length);

    //create PCB//////////////////////////////////////////////////////////////////////////////

    //determine pcb_t location based on pid
    pcb_t* pcb = (pcb_t*) (KERNEL_MEM_START - (new_pid + 1)*PROCESS_STACK_SIZE);

    //update the current active pid
    pcb->parent_pid = schedule[active_term_idx];
    schedule[active_term_idx] = new_pid;
    pcb->pid = new_pid;

    //store the kernel stack esp & ebp into pcb
    uint32_t ebp;
    asm volatile ("movl %%ebp, %0\n" :"=r"(ebp));
    pcb->kernel_ebp = ebp;
    pcb->exe_ebp = ebp;
    uint32_t esp;
    asm volatile ("movl %%esp, %0\n" :"=r"(esp));
    pcb->kernel_esp = esp;
    pcb->exe_esp = esp;
    
    setup_file_op_table();

    //set up file descriptor for each file
    for(i = 0; i < MAX_FILES; i++){
        pcb->fd_array[i].file_op_table_ptr = NULL;
        pcb->fd_array[i].inode = NULL;
        pcb->fd_array[i].file_pos = 0;
        pcb->fd_array[i].flags = 0; //not in use
    }

    //init rtc values for the process
    pcb->max_rtc_count = 0;
    pcb->rtc_interrupt = 0;
    pcb->rtc_fd_idx = -1;       //set when rtc is opened for this process

    //when process is started, automatically open stdin and stdout (fd 0, 1 respectively)
    //storing appropriate file op table and marking as in-use
    pcb->fd_array[0].file_op_table_ptr = &stdin_op;
    pcb->fd_array[0].flags = 1; 

    pcb->fd_array[1].file_op_table_ptr = &stdout_op;
    pcb->fd_array[1].flags = 1;

    //set args/////////////////////////////////////////////////////////////////////////
    for(i = 0; i < ARGS_BUF_SIZE; i++){
        pcb->args[i] = args_temp[i];  
    }
    
    //prepare for context switch///////////////////////////////////////////////////////

    // Set esp0 to be the start of the kernel memory for the process.
    tss.esp0 = KERNEL_MEM_START - ((pcb->pid)*PROCESS_STACK_SIZE) - 4; 
    tss.ss0 = KERNEL_DS;

    // Get the program entry (virtual address of the first line instruction) from the start (24-27 bytes) of the file and store as eip.
    uint8_t eip_buf[4];
    read_data(file_dentry.inode_num,EIP_START_BYTE,eip_buf,4); //eip is stored at byte 24 to 27
    int prog_eip = *((int*) eip_buf);
    pcb->user_eip = prog_eip; //save prog eip in pcb

    // The stack memery for the user program should start at the bottom of the assigned 4MB page. 
    // The page has user code on the top (low address, at 0x08048000), and stack on the bottom, growing upward.
    // Minus 4 because the reading memory from low to high address. e.g. esp = a, then popl esp get memory content from a to a + 4.
    uint32_t prog_esp = USER_MEM_START_VIR + PAGE_SIZE_4MB - 4;
    
    sti();
    // IRET is equivalent to the "popl eip, popl cs, popfl, popl esp, popl ds"
    // We use IRET to transfer control to the user program. e.g. pass the address of the user program entry as eip. 
    asm volatile(
                    "pushl %3           \n\t"   // push ds onto stack          
                    "pushl %1           \n\t"   // push esp onto stack
                    "pushfl             \n\t"   // push all flags onto stack
                    "pushl %2           \n\t"   // push cs onto stack       
                    "pushl %0           \n\t"   // push eip onto stack     
                    "iret               \n\t"   
                    :
                    : "r"(prog_eip), "r"(prog_esp), "r" (USER_CS), "r" (USER_DS)
                    : "memory"
                 );   

    sti();
    return 0;  

}

/* 
 * open
 *   DESCRIPTION: Provides access to filesystem, find dentry based on filename, locate unused fd and set up data
 *   INPUTS: filename
 *   OUTPUTS: 0 on success, -1 on failure
 */
int32_t open(const uint8_t* filename) {
    int i; // for-loop index
    if (filename == NULL || read_dentry_by_name(filename, &file_dentry) == -1) {
        return -1;
    }
    pcb_t* curr_pcb = get_pcb(schedule[active_term_idx]);
    file_descriptor_t* fd_array = curr_pcb->fd_array;
    // Find the next unused file descriptor.
    for (i = 0; i < MAX_FILES; i++) {
        if (fd_array[i].flags == FD_UNUSED) {
            fd_array[i].flags = FD_USED;
            fd_array[i].inode = file_dentry.inode_num;
            // Assign the file operation functions for each type of file.
            if (file_dentry.filetype == FILE_TYPE_RTC) {
                fd_array[i].file_op_table_ptr = &rtc_op;
                //curr_pcb->rtc_fd_idx = i;
            } else if (file_dentry.filetype == FILE_TYPE_DIR) {
                fd_array[i].file_op_table_ptr = &dir_op;
            } else {
                fd_array[i].file_op_table_ptr = &file_op;
            }
            fd_array[i].file_op_table_ptr->open(filename);
            return i;
        }
    }
    return -1; //no fd available
}

/* 
 * read
 *   DESCRIPTION: checks for valid inputs and calls the read function based on the file type
 *   INPUTS: fd, buf, nbytes
 *   OUTPUTS: 0 on success, -1 on failure
 */

int32_t read(int32_t fd, void* buf, int32_t nbytes) {
    // Check for invalid file descriptor.
    if (fd < 0 || fd >= MAX_FILES) {
        return -1;
    }
    // Check for invalid input.
    if (buf == NULL || nbytes < 0) {
        return -1;
    }
    pcb_t* curr_pcb = get_pcb(schedule[active_term_idx]);
    file_descriptor_t* fd_array = curr_pcb->fd_array;
    // Can not read from unused file descriptor
    if (fd_array[fd].flags == FD_UNUSED) {
        return -1;
    }
    return fd_array[fd].file_op_table_ptr->read(fd, buf, nbytes);
    
}

/* 
 * write
 *   DESCRIPTION: checks for valid inputs and calls the write function based on the file type
 *   INPUTS: fd, buf, nbytes
 *   OUTPUTS: 0 on success, -1 on failure
 */
int32_t write(int32_t fd, const void* buf, int32_t nbytes) {
    // Check for invalid file descriptor.
    if (fd < 0 || fd >= MAX_FILES) {
        return -1;
    }
    // Check for invalid input.
    if (buf == NULL || nbytes < 0) {
        return -1;
    }
    pcb_t* curr_pcb = get_pcb(schedule[active_term_idx]);
    file_descriptor_t* fd_array = curr_pcb->fd_array;
    // Can not write to unopened file descriptor
    if (fd_array[fd].flags == FD_UNUSED) {
        return -1;
    }
    return fd_array[fd].file_op_table_ptr->write(fd, buf, nbytes);
}

/* 
 * close
 *   DESCRIPTION: checks for valid inputs and calls the close function based on the file type
 *   INPUTS: fd
 *   OUTPUTS: 0 on success, -1 on failure
 */

int32_t close(int32_t fd) {
    // Check for invalid file descriptor.
    if (fd < 2 || fd >= MAX_FILES) {
        return -1;
    }
    pcb_t* curr_pcb = get_pcb(schedule[active_term_idx]);
    file_descriptor_t* fd_array = curr_pcb->fd_array;
    // Can not write to unopened file descriptor
    if (fd_array[fd].flags == FD_UNUSED) {
        return -1;
    }
    fd_array[fd].inode = NULL;
    fd_array[fd].file_pos = 0;
    fd_array[fd].flags = FD_UNUSED;
    return fd_array[fd].file_op_table_ptr->close(fd);
}

/* 
 * getargs
 *   DESCRIPTION: checks for valid inputs and copy args into buf
 *   INPUTS: buf, nbytes
 *   OUTPUTS: 0 on success, -1 on failure
 */
int32_t getargs(uint8_t* buf, int32_t nbytes) {
    pcb_t* curr_pcb = get_pcb(schedule[active_term_idx]);
    
    //no args
    if(curr_pcb->args[0] == '\0'){    //TODO should it be able to accept arg that begins with space?
        return -1;
    }

    //if buf is valid
    if(buf == NULL){
        return -1;
    }

    //if args is bigger than nbytes
    if(ARGS_BUF_SIZE > nbytes){
        return -1;
    }

    //copy args to buf
    memcpy(buf,&(curr_pcb->args), ARGS_BUF_SIZE);
    return 0;
}

/* 
 * vidmap
 *   DESCRIPTION: checks for valid input and store the virtual memory (0x8400000) pointing to the video memory into the buffer user provided.
 *   INPUTS: the input user provide to store the video memory start.
 *   OUTPUTS: 0 on success, -1 on failure
 */
int32_t vidmap(uint8_t** screen_start) {
    //check if screen_start is valid
    if(screen_start == NULL){
        return -1;
    }
    if ( ((uint32_t)screen_start < USER_MEM_START_VIR) || ((uint32_t)screen_start > (USER_MEM_START_VIR + PAGE_SIZE_4MB)) ) {
        return -1;
    }
    page_dir_entry_kb vid_map_pde;
    setup_page_dir_entry_kb(&vid_map_pde, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, (int)vid_map_page_table>>PAGE_TABLE_NUM_OFFSET);
    page_directory[VID_MAP_VIR >> MB_PAGE_NUM_OFFSET] = *(int*)&vid_map_pde;
    
    //check if current active process is being displayed
    uint32_t vid_mem_addr;
    if(active_term_idx != visible_term_idx){
        vid_mem_addr = VID_PAGE_T0 + (active_term_idx*PAGE_SIZE_4KB);
    }
    else{
        vid_mem_addr = VIDEO_MEMORY_START;
    }

    int i;
    for (i = 0; i < NUM_ENTRIES; i++) {
        page_table_entry table_entry;
        // The purpose is the map the virtual address 0x8400000 to the physical address at 0xB8000 (video memory)
        if (i == 0) {
            setup_page_table_entry(&table_entry, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, vid_mem_addr >> KB_PAGE_NUM_OFFSET);
        } else {
            setup_page_table_entry(&table_entry, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0);
        }
        vid_map_page_table[i] = table_entry;
    }

    //flush_tlb: processor will flush the tlb once it notices that cr3 has been changed
    asm volatile (  "movl %cr3, %eax  \n\t"
                    "movl %eax, %cr3  \n\t"      
                 );

    *screen_start = (uint8_t*) VID_MAP_VIR ;
    return 0;
}

int32_t set_handler(int32_t signum, void* handler_address) {
    return 0;
}

int32_t sigreturn() {
    return 0;
}

//stdin is a read only file
int32_t stdin_write(int32_t fd, const void* buf, int32_t nbytes){
    return -1;
}
//stdout is a write only file
int32_t stdout_read(int32_t fd, void* buf, int32_t nbytes){
    return -1;
}

//helper functions///////////////////////////////////////////////////////////////////


int32_t file_executable(const uint8_t* filename) {
    //read_dentry_by_name(filename, &file_dentry);
    uint8_t buf[4];
    uint32_t result = read_data(file_dentry.inode_num, 0, buf, 4);
    if (result == -1) {
        return -1;
    }
    uint8_t executable_header[4];
    executable_header[0] = 0x7f;
    executable_header[1] = 0x45;
    executable_header[2] = 0x4c;
    executable_header[3] = 0x46;
    if (strncmp((const int8_t *)executable_header, (const int8_t *)buf, 4) == 0) {
        return 0;
    }
    return -1;
}

/* 
 * setup_file_op_table
 *   DESCRIPTION: setup the operation table for each file type (rtc, regular file, directory).
 *   INPUTS: None.
 *   OUTPUTS: None
 */
void setup_file_op_table(){
    stdin_op.open = terminal_open;
    stdin_op.read = terminal_read;
    stdin_op.write = stdin_write;
    stdin_op.close = terminal_close;

    stdout_op.open = terminal_open;
    stdout_op.read = stdout_read;
    stdout_op.write = terminal_write;
    stdout_op.close = terminal_close;

    dir_op.open = dir_open;
    dir_op.read = dir_read;
    dir_op.write = dir_write;
    dir_op.close = dir_close;

    file_op.open = file_open;
    file_op.read = file_read;
    file_op.write = file_write;
    file_op.close = file_close;

    rtc_op.open = rtc_open;
    rtc_op.read = rtc_read;
    rtc_op.write = rtc_write;
    rtc_op.close = rtc_close;

    terminal_op.open = terminal_open;
    terminal_op.read = terminal_read;
    terminal_op.write = terminal_write;
    terminal_op.close = terminal_close;
}

pcb_t* get_pcb(uint32_t pid){
    pcb_t* pcb;
    pcb = (pcb_t*) (KERNEL_MEM_START - (pid + 1)*PROCESS_STACK_SIZE);
    return pcb;
}

void process_switch(uint32_t from_pid, uint32_t to_pid){
    pcb_t* from_pcb = get_pcb(from_pid);
    pcb_t* to_pcb = get_pcb(to_pid);

    //save esp and ebp of current terminal kernel stack
    uint32_t ebp;
    asm volatile ("movl %%ebp, %0\n" :"=r"(ebp));
    from_pcb->kernel_ebp = ebp;
    uint32_t esp;
    asm volatile ("movl %%esp, %0\n" :"=r"(esp));
    from_pcb->kernel_esp = esp;


    // Set esp0 to be the start of the kernel memory for target terminal process.
    tss.esp0 = KERNEL_MEM_START - ((to_pcb->pid)*PROCESS_STACK_SIZE) - 4; 
    tss.ss0 = KERNEL_DS;

    //maps user program to current pid in the target terminal
    setup_process_memory(to_pcb->pid);
    
    //restore ebp and esp of target terminal process
    asm volatile(   "movl %0, %%ebp         \n\t"   // restore ebp for the next process
                    "movl %1, %%esp         \n\t"
                    "leave                  \n\t"
                    "ret                    \n\t"
                    :
                    : "r"(to_pcb->kernel_ebp), "r"(to_pcb->kernel_esp)
                    : "memory"
                    );

        
}
