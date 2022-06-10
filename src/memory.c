#include "memory.h"

extern unsigned long kernel_s;
extern unsigned long kernel_e;
extern unsigned long init_pagetable_s;
extern unsigned long init_pagetable_e;
unsigned long           pagetable_cursor;

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

static int early_build_page_map(unsigned char level, unsigned long area_base, unsigned long page_table_addr, 
                                    unsigned long* phy_start, unsigned long* va_start, unsigned long* size,
                                    unsigned long pte_attr, unsigned long* pt_end)
{
    if ((*phy_start & PAGE_MASK) || (*va_start & PAGE_MASK) || (*size & PAGE_MASK) || (!*size))
        return -1;
    unsigned long* p_pti = (unsigned long*)page_table_addr;
    unsigned long pti_area_s, pti_area_e, tmp, current_table_end;
    *pt_end += PAGE_SIZE;
    current_table_end = *pt_end;
    while ((unsigned long)p_pti < current_table_end) {
        pti_area_s = area_base + ((((unsigned long)p_pti - page_table_addr)/sizeof(unsigned long)) << (12 + (3 - level)*9));
        pti_area_e = pti_area_s + (1UL << (12 + (3 - level)*9));
        if (pti_area_s <= *va_start && *va_start < pti_area_e && *size > 0) {
            if (level != 3) {
                tmp = *pt_end;
                early_build_page_map(level+1, pti_area_s, tmp, phy_start, va_start, size, pte_attr, pt_end);
                *p_pti = L02_NEXT_LEVEL_IS_PTI(tmp >> 12);
            } else {
                *p_pti = L3_PTI_SET_VALID(*phy_start >> 12, pte_attr);
                *phy_start += PAGE_SIZE;
                *size -= PAGE_SIZE;
                *va_start += PAGE_SIZE;
            }
        } else {
            if (level != 3)
                *p_pti = L02_NEXT_LEVEL_IS_INV;
            else
                *p_pti = L3_PFN_SET_INVALID;
        }
        p_pti++;
    }
    return 0;
}

static int early_modify_page_map(unsigned char level, unsigned long area_base, unsigned long page_table_addr, 
                                    unsigned long* phy_start, unsigned long* va_start, unsigned long* size,
                                    unsigned long pte_attr, unsigned long* pt_end)
{
    if ((*phy_start & PAGE_MASK) || (*va_start & PAGE_MASK) || (*size & PAGE_MASK) || (!*size))
        return -1;
    unsigned long* p_pti = (unsigned long*)page_table_addr;
    unsigned long pti_area_s, pti_area_e, tmp, current_table_end;
    *pt_end += PAGE_SIZE;
    current_table_end = *pt_end;
    p_pti += (*va_start & (0x1ff << (12 + (3-level)*9))) >> (12 + (3-level)*9);
    while ((unsigned long)p_pti < current_table_end) {
        pti_area_s = area_base + ((((unsigned long)p_pti - page_table_addr)/sizeof(unsigned long)) << (12 + (3 - level)*9));
        pti_area_e = pti_area_s + (1UL << (12 + (3 - level)*9));
        if (pti_area_s <= *va_start && *va_start < pti_area_e && *size > 0) {
            if (level != 3) {
                if ((*p_pti & 0x1) == 0) {
                    tmp = *pt_end;
                    early_build_page_map(level+1, pti_area_s, tmp, phy_start, va_start, size, pte_attr, pt_end);
                    *p_pti = L02_NEXT_LEVEL_IS_PTI(tmp >> 12);
                } else if ((*p_pti & 0x3) == 3) {
                    *pt_end -= PAGE_SIZE;
                    tmp = *p_pti & 0xfffffffff000;
                    early_modify_page_map(level+1, pti_area_s, tmp, phy_start, va_start, size, pte_attr, pt_end);
                }
            } else {
                *p_pti = L3_PTI_SET_VALID(*phy_start >> 12, pte_attr);
                *phy_start += PAGE_SIZE;
                *size -= PAGE_SIZE;
                *va_start += PAGE_SIZE;
            }
        } else {
            return 0;
        }
        p_pti++;
    }
    return 0;
}

static void build_identical_map(unsigned long* p_pgd)
{
    unsigned long phy_start, va_start, size, new_end;
    *p_pgd = init_pagetable_s;
    phy_start = MEM_ROUND_DOWN(kernel_s, PAGE_MASK);
    va_start = phy_start;
    size = init_pagetable_e - phy_start;
    new_end = *p_pgd;
    early_build_page_map(0, 0, *p_pgd, &phy_start, &va_start, &size, L3_PFN_ATTR_RWX, &new_end);
    early_dbg_print(PRT_INFO, "identical map page table from 0x%x to 0x%x!\r\n", *p_pgd, new_end);
    pagetable_cursor = new_end;
}

static void add_device_identical_map(unsigned long* p_pgd)
{
    unsigned long phy_start, va_start, size, new_end;
    phy_start = UART2_BASE;
    va_start = phy_start;
    size = UART_REG_REGION;
    new_end = pagetable_cursor;
    early_modify_page_map(0, 0, *p_pgd, &phy_start, &va_start, &size, L3_PFN_ATTR_DEV, &new_end);
    early_dbg_print(PRT_INFO, "identical device map page table from 0x%x to 0x%x!\r\n", kernel_e, new_end);
    pagetable_cursor = new_end;
}

static void mmu_reg_init(void)
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

static int mmu_enable(unsigned long pgd_addr)
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

static void reserve_page_table_mem(void)
{
    init_pagetable_s =  MEM_ROUND_UP(kernel_e, PAGE_MASK);
    pagetable_cursor = init_pagetable_s;
    unsigned long ddr_pages = (SYS_INFO_MEM_END - SYS_INFO_MEM_START) >> PAGE_SHIFT;
    unsigned long ddr_l3_pages = !(ddr_pages >> 9) ? 1 : (ddr_pages >> 9);
    unsigned long ddr_l2_pages = !(ddr_l3_pages >> 9) ? 1 : (ddr_l3_pages >> 9);
    unsigned long ddr_l1_pages = !(ddr_l2_pages >> 9) ? 1 : (ddr_l2_pages >> 9);
    unsigned long dev_pages = (SYS_INFO_DEV_END - SYS_INFO_DEV_START) >> PAGE_SHIFT;
    unsigned long dev_l3_pages = !(dev_pages >> 9) ? 1 : (dev_pages >> 9);
    unsigned long dev_l2_pages = !(dev_l3_pages >> 9) ? 1 : (dev_l3_pages >> 9);
    unsigned long dev_l1_pages = !(dev_l2_pages >> 9) ? 1 : (dev_l2_pages >> 9);
    unsigned long l0_pages = 1;
    init_pagetable_e = init_pagetable_s + (( l0_pages + dev_l1_pages + dev_l2_pages + dev_l3_pages + ddr_l1_pages + ddr_l2_pages + ddr_l3_pages ) << PAGE_SHIFT);
    early_dbg_print(PRT_INFO, "reserve memory for kernel page tables, from 0x%x to 0x%x\r\n", init_pagetable_s, init_pagetable_e);
}

void mmu_init()
{
    unsigned long pgd_identical_addr;
    reserve_page_table_mem();
    build_identical_map(&pgd_identical_addr);
    add_device_identical_map(&pgd_identical_addr);
    mmu_reg_init();
    if(0 != mmu_enable(pgd_identical_addr)) {
        early_dbg_print(PRT_ERROR, "mmu init failed!\r\n");
    }
    early_dbg_print(PRT_INFO, "mmu init success(identical map)!\r\n");
}

