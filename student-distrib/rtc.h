#ifndef _RTC_H
#define _RTC_H

#ifndef ASM

#include "i8259.h"
#include "lib.h"
#include "terminal.h"
#include "types.h"

// #define SCANCODE_SIZE   0x32


//initializes the rtc
void init_rtc();

//messes up the screen upon reception of rtc int
void rtc_handler();

//reads the status register so that rtc proceeds to give int
void read_register_C();

//CP2 System call functions. Should return 0 for success and -1 for failure

//block a flag until the next rtc inerrupt
int rtc_read(int32_t fd, void* buf, int32_t nbytes); 

//Change the rtc frequency into value in buf
int rtc_write(int32_t fd, const void* buf, int32_t nbytes); 

//initializes RTC frequency to 2Hz.
int rtc_open(const uint8_t* filename); 

//does nothing unless we virtualize RTC
int rtc_close(int32_t fd); 

//helper function to set frequency, given a rate
int rtc_set_rate(int rate); 

#endif
#endif
