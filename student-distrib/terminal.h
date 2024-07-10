#ifndef _TERMINAL_H
#define _TERMINAL_H

#include "lib.h"
#include "keyboard.h"
#include "multi_term.h"

#define KBUF_SIZE   128

// read from user buffer
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes);

// write to terminal
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes);

// open terminal (not used for ckpt2)
int32_t terminal_open(const uint8_t* filename);

// close terminal (not used for ckpt2)
int32_t terminal_close(int32_t fd);



#endif /* _TERMINAL_H */
