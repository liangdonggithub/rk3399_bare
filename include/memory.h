#ifndef MEMORY_H
#define MEMORY_H

#include "vmsa.h"
#include "early_uart.h"
#include "printf.h"

#define MEM_ROUND_DOWN(addr,mask)   (addr & (~mask))
#define MEM_ROUND_UP(addr,mask)     ((addr&mask)?((addr&(~mask))+mask+1):((!addr)?(mask+1):(addr)))

//#define L3_PFN_ATTR_RWX		        (PTI_AF_SET(1)|PTI_SH_SET(3)|PTI_PXN_SET(1)|PTI_UXN_SET(1)|PTI_DBM_SET(1)|PTI_NS_SET(1)|PTI_AttrIndx_SET(PTI_ATTRIDX_NOR))
#define L3_PFN_ATTR_RWX		        	(PTI_AF_SET(1)|PTI_SH_SET(3)|PTI_DBM_SET(1)|PTI_NS_SET(1)|PTI_AttrIndx_SET(PTI_ATTRIDX_NOR))
#define L3_PFN_ATTR_DEV					(PTI_AF_SET(1)|PTI_SH_SET(3)|PTI_DBM_SET(1)|PTI_NS_SET(1)|PTI_AttrIndx_SET(PTI_ATTRIDX_DEV_nGnRnE))
#define L3_PTI_SET_VALID(pfn, attr)		(PTI_PTYPE_SET(PTI_PTYPE_IS_VAL)|PTI_PFN_SET(pfn)|attr)
#define L3_PFN_SET_INVALID          	(PTI_PTYPE_SET(PTI_PTYPE_IS_INV))

#define L02_NEXT_LEVEL_IS_PTI(num)  (PTI_NEXTL_ADDR_SET(num)|PTI_NEXTL_TYPE_SET(PTI_NEXTL_IS_PTI))
#define L02_NEXT_LEVEL_IS_INV		(PTI_NEXTL_TYPE_SET(PTI_NEXTL_IS_INV))

#define MAIR_COMMON					((0x00UL<<(PTI_ATTRIDX_DEV_nGnRnE*8))|(0x04UL<<(PTI_ATTRIDX_DEV_nGnRE*8))|(0x0cUL<<(PTI_ATTRIDX_DEV_GRE*8))| \
									(0x44UL<<(PTI_ATTRIDX_NOR_NC*8))|(0xffUL<<(PTI_ATTRIDX_NOR*8))|(0xbbUL<<(PTI_ATTRIDX_NOR_WT*8)))

#define TCR_EL1_COMMON				(TCR_T0SZ_SET(TCR_VABITS_TO_SZ(SYS_VA_BITS))|TCR_T1SZ_SET(TCR_VABITS_TO_SZ(SYS_VA_BITS))| \
									TCR_TG0_SET(TCR_TG0_4K)|TCR_TG1_SET(TCR_TG1_4K))

void mmu_init();

void print_init_area(void);

#endif 