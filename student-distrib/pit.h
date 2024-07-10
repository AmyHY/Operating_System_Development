#ifndef _PIT_H
#define _PIT_H

#include "lib.h"
#include "i8259.h"
#include "keyboard.h"
#include "multi_term.h"
#include "syscall.h"

#define PIT_CH0             0x40        //channel 0 port
#define PIT_CMD_REG         0x43        //cmd reg port
#define INPUT_FREQ          1193182     // PIT input freq 1193182 Hz
#define PIT_FREQ            100         //set to about interrupt every 10ms -> 100Hz
#define PIT_MODE            0x36        //00|11| 010|0   channel 0 | lobyte/hibyte | mode 3 | binary
#define PIT_IRQ             0x00

int i;

void init_pit();
void pit_handler();

#endif
