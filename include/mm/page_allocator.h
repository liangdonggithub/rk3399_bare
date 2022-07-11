/*
 * @Author: Neal 624872416@qq.com
 * @Date: 2022-06-13 09:22:40
 * @LastEditors: Neal 624872416@qq.com
 * @LastEditTime: 2022-07-11 09:31:12
 * @FilePath: /rk3399_bare/include/mm/page_allocator.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef PAGE_ALLOCATOR_H
#define PAGE_ALLOCATOR_H

#include "sysinfo.h"
#include "sysinfo_c.h"
#include "mm/page.h"
#include "mm/mmu.h"

#define pfn_to_p_idx(pfn)						((pfn) - (SYS_INFO_MEM_START >> PAGE_SHIFT))

#define MAX_ORDER            10

#if SYS_INFO_MEM_SIZE < 0x100000000UL
#define PAGE_ORDER_SHIFT     20
#define PAGE_ATTR0_SHIFT     24
#define PAGE_SUBS_SHIFT		 20
#define PAGE_ATTR1_SHIFT     26
#else
#define PAGE_ORDER_SHIFT     36
#define PAGE_ATTR0_SHIFT     40
#define PAGE_SUBS_SHIFT		 36
#define PAGE_ATTR1_SHIFT     42
#endif

#define PAGE_ATTR_LIST_HEAD                     (1<<0)
#define PAGE_ATTR_LIST_LAST                     (1<<1)
#define PAGE_ATTR_USED                          (1<<2)
#define PAGE_ATTR_ORDER_GROUP_HEADER            (1<<3)

enum page_map_type{
	MAP_NOT = 0,
	MAP_DIRECT,
	MAP_ASSIGNED_VA,
};

struct page_order {
    struct page             list_head;
    u64                     counter;
};

struct page_type {
    struct page_order       orders[MAX_ORDER + 1];
};

struct page_area {
    struct page_type        used[SUBS_MAX];
    struct page_type        unused;
};

int init_page_allocator(struct page* pageitembase);

struct page* alloc_pages(u8 order, enum page_map_type type, u64 va, enum SUB_SYSTEM subs);

void* alloc_pages_discrete(u64 pages, u64 va, enum SUB_SYSTEM subs);

void release_pages(u64 va, u64 order);

void release_pages_discrete(u64 va, u64 pages);

#endif