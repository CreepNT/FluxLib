//This header is a temporary and incomplete implementation of the FluxConsole library.


#ifndef __TEMPORARY__LOG__H
#define __TEMPORARY__LOG__H
#include <psp2/kernel/clib.h>
#pragma GCC warning "stub.h is in use !\nThis header is temporary and its API should be considered unstable.\nPlease move to FluxLib API as soon as possible.\n"

#pragma region ANSI Escape codes
#define TERM_RED "\\u001b[31m"
#define TERM_GREEN "\\u001b[32m"
#define TERM_YELLOW "\\u001b[33m"
#define TERM_BLUE "\\u001b[34m"
#define TERM_MAGENTA "\\u001b[35m"
#define TERM_CYAN "\\u001b[36m"
#define TERM_WHITE "\\u001b[37m"
#define TERM_RESET "\\u001b[0m"
//Those codes are only supported by terminals with 16 colors
#pragma end

#define LOG_ERROR(fmt, ...) sceClibPrintf("\n%s(%d):" TERM_RED " Error: " fmt TERM_RESET ,__FILE__, __LINE__,##__VA_ARGS__)
#define LOG_INFO(fmt, ...)  sceClibPrintf(fmt, ##__VA_ARGS__);


#endif