#ifndef PAGE_ALLOCATOR_H
#define PAGE_ALLOCATOR_H

#include "tools/list.h"
#include "tools/octree.h"

#define PAGE_LEVEL_CLEAR        0
#define PAGE_LEVEL_PART         1
#define PAGE_LEVEL_FULL         2
#define PAGE_LEVEL_SET_USED(val,idx,used)   do {val &= ~(0x3<<(idx*2)); val |= ((x & 0x3)<<(idx*2));} while(0)
#define PAGE_LEVEL_GET_USED(val,idx)        ((val >> (idx*2)) & 0x3)
#define PAGE_LEVEL_SET_LEVEL(val,level)     do {val &= ~(0xf<<16); val |= ((level & 0xf) << 16);} while(0)
#define PAGE_LEVEL_GET_LEVEL(val)           ((val >> 16) & 0xf)

struct page_item {
    struct list_head        list;
    unsigned long           attrs;
};

struct page_level {
    union
    {
        struct octree_item  oct_node;
        struct page_item*   page_nodes[8];
    }next_level;
    unsigned long           attrs;
};

#endif