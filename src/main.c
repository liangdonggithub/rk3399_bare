#include "printf.h"
#include "dev/early_uart.h"
#include "sysinfo.h"
#include "mm/mmu.h"
#include "mm/malloc.h"

unsigned long kernel_s;
unsigned long kernel_e;
unsigned long init_pagetable_s;
unsigned long init_pagetable_e;
unsigned long init_stack_s;
unsigned long init_stack_e;
unsigned long init_heap_s;
unsigned long init_heap_e;

extern void jump_to_kern(unsigned long kern_shift, unsigned long kern_sp);

int main(void)
{
    early_uart_init();
    early_dbg_print(PRT_INFO, "kernel text ==============>\r\n");
    print_init_area();
    init_addr_lds(area);
    kernel_s = area_s;
    kernel_e = area_e;
    mmu_init();
    jump_to_kern(KERN_NORMAL_REGION_S, KERN_STACK_REGION_S);
    early_dbg_print(PRT_INFO, "enter kernel space!\r\n");
    if (init_malloc_sys(KERN_HEAP_REGION_S, PAGE_SIZE*KERN_INIT_HEAP_PAGES)) {
        early_dbg_print(PRT_INFO, "malloc system init success\r\n");
    } else {
        early_dbg_print(PRT_INFO, "malloc system init failed!\r\n");
        goto halt;
    }
    char* heap_str = malloc(20);
    char* heap_str1 = malloc(20);
    free(heap_str1);
    free(heap_str);
halt:
    while(1){};
}

