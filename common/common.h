#ifndef __COMMON_H
#define __COMMON_H

//Breakpoint. Will crash app on Retail (and Testkit ?)
#define BREAK() __asm__ volatile("bkpt 0x0000")

#endif