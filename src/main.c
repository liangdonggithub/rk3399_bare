/*
 * @Author: Neal 624872416@qq.com
 * @Date: 2022-06-02 11:25:17
 * @LastEditors: Neal 624872416@qq.com
 * @LastEditTime: 2022-07-06 15:37:35
 * @FilePath: /rk3399_bare/src/main.c
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "printf.h"
#include "dev/early_uart.h"
#include "sysinfo.h"
#include "mm/mmu.h"
#include "mm/malloc.h"
#include "mm/page_allocator.h"

u64 kernel_phy_s;
u64 kernel_phy_e;
u64 early_page_table_phy_s;
u64 early_page_table_phy_e;
u64 early_stack_phy_s;
u64 early_stack_phy_e;
u64 page_item_phy_s;
u64 page_item_phy_e;

extern void jump_to_kern(unsigned long kern_shift, unsigned long kern_sp);


void print_init_area()
{
    init_addr_lds(area);
    init_addr_lds(text);
    init_addr_lds(rodata);
    init_addr_lds(data);
    init_addr_lds(bss);
    init_addr_lds(reladyn);
    init_addr_lds(stack);
    init_addr_lds(vector);
    early_dbg_print(PRT_INFO, "kernel init area from 0x%x to 0x%x size %d bytes:\r\n", area_s, area_e, area_e - area_s);
    early_dbg_print(PRT_INFO, "+---------+------------+------------+------------+\r\n");
    early_dbg_print(PRT_INFO, "|name     |    start(x)|      end(x)|     size(d)|\r\n");
    early_dbg_print(PRT_INFO, "+---------+------------+------------+------------+\r\n");
    early_dbg_print(PRT_INFO, "|text     |%12x|%12x|%12d|\r\n", text_s, text_e, text_e - text_s);
    early_dbg_print(PRT_INFO, "|rodata   |%12x|%12x|%12d|\r\n", rodata_s, rodata_e, rodata_e - rodata_s);
    early_dbg_print(PRT_INFO, "|data     |%12x|%12x|%12d|\r\n", data_s, data_e, data_e - data_s);
    early_dbg_print(PRT_INFO, "|bss      |%12x|%12x|%12d|\r\n", bss_s, bss_e, bss_e - bss_s);
    early_dbg_print(PRT_INFO, "|rela.dyn |%12x|%12x|%12d|\r\n", reladyn_s, reladyn_e, reladyn_e - reladyn_s);
    early_dbg_print(PRT_INFO, "|stack    |%12x|%12x|%12d|\r\n", stack_s, stack_e, stack_e - stack_s);
    early_dbg_print(PRT_INFO, "|vector   |%12x|%12x|%12d|\r\n", vector_s, vector_e, vector_e - vector_s);
    early_dbg_print(PRT_INFO, "+---------+------------+------------+------------+\r\n");
}

void heap_test()
{
    char* heap_str = malloc(20);
    char* heap_str1 = malloc(20);
    heap_str[0] = 'h';
    heap_str[1] = 'e';
    heap_str[2] = 'l';
    heap_str[3] = 'l';
    heap_str[4] = 'o';
    heap_str[5] = '!';
    heap_str[6] = 0;
    heap_str1[0] = 'w';
    heap_str1[1] = 'o';
    heap_str1[2] = 'r';
    heap_str1[3] = 'l';
    heap_str1[4] = 'd';
    heap_str1[5] = '!';
    heap_str1[6] = 0;
    early_dbg_print(PRT_INFO, "malloc test str:%s str1:%s\r\n", heap_str, heap_str1);
    free(heap_str1);
    free(heap_str);
}


int main(void)
{
    early_uart_init();
    early_dbg_print(PRT_INFO, "kernel text ==============>\r\n");
    print_init_area();
    init_addr_lds(area);
    kernel_phy_s = area_s;
    kernel_phy_e = MEM_ROUND_UP(area_e, PAGE_MASK);
    mmu_init();
    jump_to_kern(KERN_NORMAL_REGION_S, KERN_STACK_REGION_S);
    early_dbg_print(PRT_INFO, "enter kernel address space\r\n");
    init_page_allocator((struct page*)get_linear_va_by_pa(page_item_phy_s));
    early_dbg_print(PRT_INFO, "page allocator init success\r\n");
    if (init_malloc_sys()) {
        early_dbg_print(PRT_INFO, "malloc system init success\r\n");
    } else {
        early_dbg_print(PRT_INFO, "malloc system init failed!\r\n");
        goto halt;
    }
    heap_test();
halt:
    while(1){};
}

