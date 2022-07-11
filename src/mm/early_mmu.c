/*
 * @Author: Neal 624872416@qq.com
 * @Date: 2022-07-04 16:45:56
 * @LastEditors: Neal 624872416@qq.com
 * @LastEditTime: 2022-07-06 10:57:22
 * @FilePath: /rk3399_bare/src/mm/early_mmu.c
 * @Description: mmu init and memory area reserve, only invoked in kernel boot
 */

#include "mm/mmu.h"

#define get_levelx_idx(vfn, l)          (((vfn) >> (9 * (3 - (l)))) & 0x1ff)

#define get_nextl_pfn_from_pti(p)		((*(p) >> 12) & 0xfffffffffUL)

#define get_nextl_addr_from_pti(p)		((void*)(get_nextl_pfn_from_pti(p) << PAGE_SHIFT))

#define get_pt_02_nextl_type(p)			(*(p) & 0x3)

#define set_pt_02_nextl_type(p,t)       do {    \
                                            *(p) &= 0xffffffffffffffffUL << 2;  \
                                            *(p) |= (t & 0x3);  \
                                        }while(0)

#define get_pt_3_page_type(p)			(*(p) & 0x3)

#define set_pt_3_page_type(p,t)			do {	\
											*(p) &= 0xffffffffffffffffUL << 2;	\
											*(p) |= (t & 0x3);	\
										}while(0)

extern u64				kernel_phy_s;
extern u64				kernel_phy_e;
extern u64				early_page_table_phy_s;
extern u64				early_page_table_phy_e;
extern u64				early_stack_phy_s;
extern u64				early_stack_phy_e;
extern u64				page_item_phy_s;
extern u64				page_item_phy_e;

u64*					g_kernel_pt_root;
static u64              unuse_pfn;

/**
 * @description: reserve init stack memory, only used in kernel init
 * @param {u64} start_phy_addr
 * @return {*}
 */
static void __early_reserve_stack_mem(u64 start_phy_addr)
{
    early_stack_phy_s = MEM_ROUND_UP(start_phy_addr, PAGE_MASK);
    early_stack_phy_e = early_stack_phy_s + PAGE_SIZE*KERN_INIT_STACK_PAGES;
}

/**
 * @description: reserve struct page item for ddr, only used in kernel init
 * @param {u64} start_phy_addr
 * @return {*}
 */
static void __early_reserve_page_item_mem(u64 start_phy_addr)
{
    page_item_phy_s = MEM_ROUND_UP(start_phy_addr, PAGE_MASK);
    page_item_phy_e = MEM_ROUND_UP(page_item_phy_s + (SYS_INFO_MEM_SIZE>>PAGE_SHIFT) * sizeof(struct page), PAGE_MASK);
}

/**
 * @description: page allocator add by self
 * @return {*}
 */
static u64* __early_alloc_page()
{
    return (u64*)((unuse_pfn++) << PAGE_SHIFT);
}

/**
 * @description: init l0 page table
 * @return {*}
 */
static void __early_init_l0_page_table()
{
	int tmp;
    g_kernel_pt_root = __early_alloc_page();
	early_page_table_phy_s = (u64)g_kernel_pt_root;
    for (tmp = 0; tmp < PAGE_TABLE_ITEM; tmp++) {
        set_pt_02_nextl_type(&g_kernel_pt_root[tmp], PTI_NEXTL_IS_INV);
    }
}

static void __early_alloc_l3_page_table(u64* higher, u64 pfn, u64 vfn, u64 attr)
{
	u64* page = __early_alloc_page();
	int tmp;
    for (tmp = 0; tmp < PAGE_TABLE_ITEM; tmp++) {
		set_pt_3_page_type(&page[tmp], PTI_PTYPE_IS_INV);
    }
	*higher = L02_NEXT_LEVEL_IS_PTI((u64)page >> PAGE_SHIFT);
	u16 l3 = get_levelx_idx(vfn, 3);
	page[l3] = L3_PTI_SET_VALID(pfn, attr);
}

static void __early_alloc_l2_page_table(u64* higher, u64 pfn, u64 vfn, u64 attr)
{
	u64* page = __early_alloc_page();
	int tmp;
    for (tmp = 0; tmp < PAGE_TABLE_ITEM; tmp++) {
        set_pt_02_nextl_type(&page[tmp], PTI_NEXTL_IS_INV);
    }
	*higher = L02_NEXT_LEVEL_IS_PTI((u64)page >> PAGE_SHIFT);
	u16 l2 = get_levelx_idx(vfn, 2);
	__early_alloc_l3_page_table(&page[l2], pfn, vfn, attr);
}

static void __early_alloc_l1_page_table(u64* higher, u64 pfn, u64 vfn, u64 attr)
{
	u64* page = __early_alloc_page();
	int tmp;
    for (tmp = 0; tmp < PAGE_TABLE_ITEM; tmp++) {
        set_pt_02_nextl_type(&page[tmp], PTI_NEXTL_IS_INV);
    }
	*higher = L02_NEXT_LEVEL_IS_PTI((u64)page >> PAGE_SHIFT);
	u16 l1 = get_levelx_idx(vfn, 1);
	__early_alloc_l2_page_table(&page[l1], pfn, vfn, attr);
}

/**
 * @description: alloc or modify page table
 * @param {u64} pfn
 * @param {u64} vfn
 * @param {u64} pages
 * @param {u64} attr
 * @return {*}
 */
static void __early_map(u64 pfn, u64 vfn, u64 pages, u64 attr)
{
	u64* page;
    u16 l0, l1, l2, l3;
    while (pages) {
        l0 = get_levelx_idx(vfn, 0);
		if (PTI_NEXTL_IS_INV == get_pt_02_nextl_type(&g_kernel_pt_root[l0])) {
			__early_alloc_l1_page_table(&g_kernel_pt_root[l0], pfn, vfn, attr);
			pages--;
			pfn++;
			vfn++;
			continue;
		}
		page = get_nextl_addr_from_pti(&g_kernel_pt_root[l0]);
		l1 = get_levelx_idx(vfn, 1);
		if (PTI_NEXTL_IS_INV == get_pt_02_nextl_type(&page[l1])) {
			__early_alloc_l2_page_table(&page[l1], pfn, vfn, attr);
			pages--;
			pfn++;
			vfn++;
			continue;
		}
		page = get_nextl_addr_from_pti(&page[l1]);
		l2 = get_levelx_idx(vfn, 2);
		if (PTI_NEXTL_IS_INV == get_pt_02_nextl_type(&page[l2])) {
			__early_alloc_l3_page_table(&page[l2], pfn, vfn, attr);
			pages--;
			pfn++;
			vfn++;
			continue;
		}
		page = get_nextl_addr_from_pti(&page[l2]);
		l3 = get_levelx_idx(vfn, 3);
		page[l3] = L3_PTI_SET_VALID(pfn, attr);
		pages--;
		pfn++;
		vfn++;
    }
}

static void __mmu_reg_init(void)
{
    unsigned long tmp;
    unsigned long tcr = TCR_EL1_COMMON;
    asm("tlbi vmalle1");
    asm("dsb nsh");
    write_sysreg(3UL << 20, cpacr_el1);
    write_sysreg(1 << 12, mdscr_el1);
    write_sysreg(MAIR_COMMON, mair_el1);
    tmp = read_sysreg(ID_AA64MMFR0_EL1);
    tmp &= 0xf;
    if (tmp > ID_AA64MMFR0_PARANGE_48)
        tmp = ID_AA64MMFR0_PARANGE_48;
    tcr |= TCR_IPS_SET(tmp);
    write_sysreg(tcr, tcr_el1);
}

static int __mmu_enable(unsigned long pgd_addr)
{
    unsigned long tmp;
    tmp = read_sysreg(ID_AA64MMFR0_EL1);
    tmp = ID_AA64MMFR0_GET_TGran4(tmp);
    if (tmp != ID_AA64MMFR0_TGran4_Spt)
        return -1;
    write_sysreg(pgd_addr, ttbr0_el1);
    asm("isb");
    write_sysreg(pgd_addr, ttbr1_el1);
    asm("isb");
    tmp = read_sysreg(sctlr_el1);
    tmp |= SCTLR_ELx_M;
    write_sysreg(tmp, sctlr_el1);
    asm("isb");
    asm("ic iallu");
    asm("dsb nsh");
    asm("isb");
    return 0;
}

void mmu_init()
{
    __early_reserve_stack_mem(kernel_phy_e);
    __early_reserve_page_item_mem(early_stack_phy_e);
    //page table pfn follow reserve area
    unuse_pfn = page_item_phy_e >> PAGE_SHIFT;
	__early_init_l0_page_table();

	//identical map for kernel context
	__early_map(kernel_phy_s>>PAGE_SHIFT, kernel_phy_s>>PAGE_SHIFT, 
		(kernel_phy_e>>PAGE_SHIFT) - (kernel_phy_s>>PAGE_SHIFT), L3_PFN_ATTR_RWX);
	early_dbg_print(PRT_INFO, "map kernel context from 0x%x-0x%x to 0x%x-0x%x(0xffff%012x-0xffff%012x)\r\n",
		kernel_phy_s, kernel_phy_e, kernel_phy_s, kernel_phy_e, kernel_phy_s, kernel_phy_e);
	//identical map for debug uart device
	__early_map(UART2_BASE>>PAGE_SHIFT, UART2_BASE>>PAGE_SHIFT, 
		UART_REG_REGION>>PAGE_SHIFT, L3_PFN_ATTR_DEV);
	early_dbg_print(PRT_INFO, "map uart2 device memory from 0x%x-0x%x to 0x%x-0x%x(0xffff%012x-0xffff%012x)\r\n",
		UART2_BASE, UART2_BASE + UART_REG_REGION, UART2_BASE, UART2_BASE + UART_REG_REGION, 
		UART2_BASE, UART2_BASE + UART_REG_REGION);
	//map for stack
	__early_map(early_stack_phy_s>>PAGE_SHIFT, 
		((KERN_STACK_REGION_S & 0x0000ffffffffffffUL) - KERN_INIT_STACK_PAGES*PAGE_SIZE) >> PAGE_SHIFT,
		(early_stack_phy_e>>PAGE_SHIFT) - (early_stack_phy_s>>PAGE_SHIFT), L3_PFN_ATTR_RWX);
	early_dbg_print(PRT_INFO, "map kernel work stack from 0x%lx-0x%lx to 0x%lx-0x%lx\r\n",
		early_stack_phy_s, early_stack_phy_e, KERN_STACK_REGION_S - KERN_INIT_STACK_PAGES*PAGE_SIZE, KERN_STACK_REGION_S);
	//linear map for all ddr memory
	__early_map(SYS_INFO_MEM_START >> PAGE_SHIFT, 
		((KERN_LINEAR_REGION_S & 0x0000ffffffffffffUL) + SYS_INFO_MEM_START) >> PAGE_SHIFT,
		(SYS_INFO_MEM_END>>PAGE_SHIFT) - (SYS_INFO_MEM_START>>PAGE_SHIFT),
		L3_PFN_ATTR_RWX);
	early_dbg_print(PRT_INFO, "map all ddr memory from 0x%lx-0x%lx to 0x%lx-0x%lx\r\n",
		SYS_INFO_MEM_START, SYS_INFO_MEM_END, KERN_LINEAR_REGION_S + SYS_INFO_MEM_START, KERN_LINEAR_REGION_S + SYS_INFO_MEM_END);
	//linear map for all dev memory
	__early_map(SYS_INFO_DEV_START >> PAGE_SHIFT, 
		((KERN_LINEAR_REGION_S & 0x0000ffffffffffffUL) + SYS_INFO_DEV_START) >> PAGE_SHIFT,
		(SYS_INFO_DEV_END>>PAGE_SHIFT) - (SYS_INFO_DEV_START>>PAGE_SHIFT),
		L3_PFN_ATTR_DEV);
	early_dbg_print(PRT_INFO, "map all device memory from 0x%lx-0x%lx to 0x%lx-0x%lx\r\n",
		SYS_INFO_DEV_START, SYS_INFO_DEV_END, KERN_LINEAR_REGION_S + SYS_INFO_DEV_START, KERN_LINEAR_REGION_S + SYS_INFO_DEV_END);

	early_page_table_phy_e = unuse_pfn << PAGE_SHIFT;
	
    __mmu_reg_init();
    if(0 != __mmu_enable((u64)g_kernel_pt_root)) {
        early_dbg_print(PRT_ERROR, "mmu init failed!\r\n");
    }
    early_dbg_print(PRT_INFO, "mmu init success!\r\n");
}
