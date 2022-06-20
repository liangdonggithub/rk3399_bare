#ifndef SYSINFO_H
#define SYSINFO_H

#define GLOBAL_PRINT_LEVEL      0

//#define SYS_INFO_ALLOW_SWJ 
#define SYS_INFO_MEM_START      0x0UL
#define SYS_INFO_MEM_END        0x40000000UL
#define SYS_INFO_DEV_START      0xf8000000UL
#define SYS_INFO_DEV_END        0x100000000UL
//memory page
#define SYS_VA_BITS             48
#define PAGE_SIZE               4096
#define PAGE_SHIFT              12
#define PAGE_TABLE_ITEM         512
#define PAGE_MASK               0xfff
//va plan
#define KERN_NORMAL_REGION_S         0xffff000000000000UL
#define KERN_HEAP_REGION_S           0xffff000080000000UL
#define KERN_STACK_REGION_S          0xfffffffffffff000UL
#define KERN_INIT_STACK_PAGES        4
#define KERN_INIT_HEAP_PAGES         16

//lds related
#define init_addr_lds(x) extern int __init_##x##_s, __init_##x##_e; \
 unsigned long x##_s = (unsigned long)&__init_##x##_s; unsigned long x##_e = (unsigned long)&__init_##x##_e;

#endif