#ifndef PRINTF_H
#define PRINTF_H

#define  __out_putchar  early_uart_sendbyte

#define  MAX_NUMBER_BYTES  64
extern int printf_test(void);
extern int printf(const char *fmt, ...);
extern void puts(char *ptr);
#endif