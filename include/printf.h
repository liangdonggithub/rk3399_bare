/*
 * @Author: Neal 624872416@qq.com
 * @Date: 2022-06-01 17:22:08
 * @LastEditors: Neal 624872416@qq.com
 * @LastEditTime: 2022-07-06 09:15:42
 * @FilePath: /rk3399_bare/include/printf.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef PRINTF_H
#define PRINTF_H

#include "sysinfo.h"

#define  __out_putchar  early_uart_sendbyte

#define  MAX_NUMBER_BYTES  64

enum PRT_LEVEL{
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