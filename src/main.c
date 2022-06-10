#include "printf.h"
#include "early_uart.h"
#include "sysinfo.h"
#include "memory.h"

unsigned long kernel_s;
unsigned long kernel_e;
unsigned long init_pagetable_s;
unsigned long init_pagetable_e;

int main(void)
{
    early_uart_init();
    printf("enter kernel!\r\n");
    print_init_area();
    init_addr_lds(area);
    kernel_s = area_s;
    kernel_e = area_e;
    mmu_init();

    while(1){};
}

