/*
 * @Author: Neal 624872416@qq.com
 * @Date: 2022-07-06 09:10:17
 * @LastEditors: Neal 624872416@qq.com
 * @LastEditTime: 2022-07-07 16:34:38
 * @FilePath: /rk3399_bare/include/mm/page.h
 * @Description: page related data struct
 */
#ifndef PAGE_H
#define PAGE_H

#include "sysinfo.h"
#include "tools/types.h"

#define get_linear_va_by_pa(pa)                    ((pa) + KERN_LINEAR_REGION_S)
#define get_pa_by_linear_va(va)                    ((va) - KERN_LINEAR_REGION_S)
#define va_in_linear_area(va)                      ((((va) >= KERN_LINEAR_REGION_S) && ((va) < KERN_HEAP_REGION_S)) ? 1 : 0)

struct page {
#if SYS_INFO_MEM_SIZE < 0x100000000UL
    u32                     attr0_order_prevpidx;
    u32                     attr1_subs_nextpidx;
#else
    u64                     attr0_order_prevpidx;
    u64                     attr1_subs_nextpidx;
#endif
};

#endif