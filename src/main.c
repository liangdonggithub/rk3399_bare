#include "printf.h"
#include "early_uart.h"
int main(void)
{
    early_uart_init();
    printf("hello world!\n");
    while(1){};
}