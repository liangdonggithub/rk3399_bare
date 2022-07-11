/*
 * @Author: Neal 624872416@qq.com
 * @Date: 2022-06-02 14:38:04
 * @LastEditors: Neal 624872416@qq.com
 * @LastEditTime: 2022-07-07 17:41:52
 * @FilePath: /rk3399_bare/include/mm/mmu.h
 * @Description: mmu related
 */
#ifndef MEMORY_H
#define MEMORY_H

#include "mm/vmsa.h"
#include "dev/early_uart.h"
#include "mm/page.h"
#include "printf.h"
#include "tools/helper.h"
#include "tools/types.h"

//#define L3_PFN_ATTR_RWX		        (PTI_AF_SET(1)|PTI_SH_SET(3)|PTI_PXN_SET(1)|PTI_UXN_SET(1)|PTI_DBM_SET(1)|PTI_NS_SET(1)|PTI_AttrIndx_SET(PTI_ATTRIDX_NOR))
#define L3_PFN_ATTR_RWX		        	(PTI_AF_SET(1)|PTI_SH_SET(3)|PTI_DBM_SET(1)|PTI_NS_SET(1)|PTI_AttrIndx_SET(PTI_ATTRIDX_NOR))
#define L3_PFN_ATTR_DEV					(PTI_AF_SET(1)|PTI_SH_SET(3)|PTI_DBM_SET(1)|PTI_NS_SET(1)|PTI_AttrIndx_SET(PTI_ATTRIDX_DEV_nGnRnE))
#define L3_PTI_SET_VALID(pfn, attr)		(PTI_PTYPE_SET(PTI_PTYPE_IS_VAL)|PTI_PFN_SET(pfn)|attr)
#define L3_PFN_SET_INVALID          	(PTI_PTYPE_SET(PTI_PTYPE_IS_INV))

#define L02_NEXT_LEVEL_IS_PTI(num)  	(PTI_NEXTL_ADDR_SET(num)|PTI_NEXTL_TYPE_SET(PTI_NEXTL_IS_PTI))
#define L02_NEXT_LEVEL_IS_INV			(PTI_NEXTL_TYPE_SET(PTI_NEXTL_IS_INV))

#define MAIR_COMMON						((0x00UL<<(PTI_ATTRIDX_DEV_nGnRnE*8))|(0x04UL<<(PTI_ATTRIDX_DEV_nGnRE*8))|(0x0cUL<<(PTI_ATTRIDX_DEV_GRE*8))| \
										(0x44UL<<(PTI_ATTRIDX_NOR_NC*8))|(0xffUL<<(PTI_ATTRIDX_NOR*8))|(0xbbUL<<(PTI_ATTRIDX_NOR_WT*8)))

#define TCR_EL1_COMMON					(TCR_T0SZ_SET(TCR_VABITS_TO_SZ(SYS_VA_BITS))|TCR_T1SZ_SET(TCR_VABITS_TO_SZ(SYS_VA_BITS))| \
										TCR_TG0_SET(TCR_TG0_4K)|TCR_TG1_SET(TCR_TG1_4K))

#define MAP_STATUS_REMAPED				(1<<0)
#define MAP_STATUS_NOTMAPPED			(1<<16)
#define MAP_STATUS_ABORT				(1<<31)

#define INVALID_PFN						(0x8000000000000000UL)

extern u64*								g_kernel_pt_root;

typedef u64* (*page_table_alloc)(void);

typedef void (*page_table_release)(u64);

void mmu_init();

u64 map_pages(u64 pfn, u64 vfn, u64 pages, u64 attr, u64* pt_root_va, page_table_alloc pta);

u64 unmap_pages(u64 vfn, u64 pages, u64* pt_root_va, page_table_release ptr);

u64 mapped_pfn(u64 vfn, u64* pt_root_va);


#endif 