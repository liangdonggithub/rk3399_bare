#ifndef PRINTF_H
#define PRINTF_H

#include "sysinfo.h"

#define  __out_putchar  early_uart_sendbyte

#define  MAX_NUMBER_BYTES  64

enum {
    PRT_DEBUG = 0,
    PRT_INFO,
    PRT_WARNING,
    PRT_ERROR,
};

int printf(const char *fmt,...);

#define early_dbg_print(level,fmt, ...) { \
    if ( level >= GLOBAL_PRINT_LEVEL ) \
        printf(fmt,##__VA_ARGS__); \
}
//extern void puts(char *ptr);
#endif