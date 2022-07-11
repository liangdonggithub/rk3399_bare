/*
 * @Author: Neal 624872416@qq.com
 * @Date: 2022-07-06 09:17:33
 * @LastEditors: Neal 624872416@qq.com
 * @LastEditTime: 2022-07-11 09:17:16
 * @FilePath: /rk3399_bare/src/mm/mmu_map.c
 * @Description: mmu page map function. used by subsystems
 */
#include "mm/mmu.h"

#define get_levelx_idx(vfn, l)          (((vfn) >> (9 * (3 - (l)))) & 0x1ff)
#define get_level3_type(val)            ((val) & 0x3)
#define get_level02_type(val)			((val) & 0x3)
#define get_nextl_pfn_from_pti(val)		(((val) >> 12) & 0xfffffffffUL)
#define get_level3_pfn(val)				(((val) >> 12) & 0xfffffffffUL)
#define get_nextl_pt_linear_va(val)		(get_linear_va_by_pa(get_nextl_pfn_from_pti(val) << PAGE_SHIFT))

#define set_pti_pfn(val,pfn)			do {	\
											(val) &= ~(0xfffffffffUL << 12);	\
											(val) |= ((pfn) & 0xfffffffffUL) << 12;	\
										}while(0)

#define set_l3_pti_attr(val,attr)		do {	\
											(val) &= ~(0x1fffUL << 51);	\
											(val) &= ~(0x7ffUL << 2);	\
											(val) |= attr;	\
										}while(0)

#define set_pti_type(val,t)				do {	\
											(val) &= ~0x3UL;	\
											(val) |= (t) & 0x3UL;	\
										}while(0)

static void __page_table_clr(u64* pt)
{
	u16 idx = 0;
	for (idx = 0; idx < PAGE_TABLE_ITEM; idx++) {
		pt[idx] = 0;
	}
}

static u16 __get_pt_lev3_used(u64* pt)
{
	u16 count = 0;
	count |= (pt[0] >> 48) & 0x7UL;
	count |= ((pt[1] >> 48) & 0x7UL) << 3;
	count |= ((pt[2] >> 48) & 0x7UL) << 6;
	count |= ((pt[3] >> 48) & 0x7UL) << 9;
	return count;
}

static void __set_pt_lev3_used(u64* pt, u16 count)
{
	pt[0] &= ~(0x7UL << 48);
	pt[1] &= ~(0x7UL << 48);
	pt[2] &= ~(0x7UL << 48);
	pt[3] &= ~(0x7UL << 48);
	pt[0] |= (count & 0x7UL) << 48;
	pt[1] |= ((count >> 3) & 0x7UL) << 48;
	pt[2] |= ((count >> 6) & 0x7UL) << 48;
	pt[3] |= ((count >> 9) & 0x7UL) << 48;
}

static u16 __lev3_self_plus(u64* pt)
{
	u16 count = __get_pt_lev3_used(pt);
	__set_pt_lev3_used(pt, count + 1);
	return count + 1;
}

static u16 __lev3_self_minus(u64* pt)
{
	u16 count = __get_pt_lev3_used(pt);
	__set_pt_lev3_used(pt, count - 1);
	return count - 1;
}

static u16 __get_pt_lev02_used(u64* pt)
{
	u16 count = (pt[0] >> 2) & 0x3ff;
}

static void __set_pt_lev02_used(u64* pt, u16 count)
{
	pt[0] &= ~(0x3ffUL << 2);
	pt[0] |= (count & 0x3ffUL) << 2;
}

static u16 __lev02_self_plus(u64* pt)
{
	u16 count = __get_pt_lev02_used(pt);
	__set_pt_lev02_used(pt, count + 1);
	return count + 1;
}

static u16 __lev02_self_minus(u64* pt)
{
	u16 count = __get_pt_lev02_used(pt);
	__set_pt_lev02_used(pt, count - 1);
	return count - 1;
}

static u64 __search_add_lx_page_table(u64 pfn, u64 vfn, u8 lev, u64 attr, u64* pt_va, page_table_alloc pta)
{
    if (lev == 3) {
        u16 l3idx = get_levelx_idx(vfn, 3);
        if (PTI_PTYPE_IS_INV == get_level3_type(pt_va[l3idx])) {
			set_pti_pfn(pt_va[l3idx], pfn);
			set_l3_pti_attr(pt_va[l3idx], attr);
			set_pti_type(pt_va[l3idx], PTI_PTYPE_IS_VAL);
			__lev3_self_plus(pt_va);
            return 0;
        } else {
            set_pti_pfn(pt_va[l3idx], pfn);
			set_l3_pti_attr(pt_va[l3idx], attr);
			set_pti_type(pt_va[l3idx], PTI_PTYPE_IS_VAL);
            return MAP_STATUS_REMAPED;
        }
    } else {
        u16 lxidx = get_levelx_idx(vfn, lev);
        if (L02_NEXT_LEVEL_IS_INV != get_level02_type(pt_va[lxidx])) {
			u64 next_lev_pt_va = get_nextl_pt_linear_va(pt_va[lxidx]);
			return __search_add_lx_page_table(pfn, vfn, lev + 1, attr, (u64*)next_lev_pt_va, pta);
		} else {
			u64* next_lev_pt = pta();
			if (!next_lev_pt)
				return MAP_STATUS_ABORT;
			__page_table_clr(next_lev_pt);
			set_pti_pfn(pt_va[lxidx], get_pa_by_linear_va((u64)next_lev_pt)>>PAGE_SHIFT);
			set_pti_type(pt_va[lxidx], PTI_NEXTL_IS_PTI);
			__lev02_self_plus(pt_va);
			return __search_add_lx_page_table(pfn, vfn, lev + 1, attr, next_lev_pt, pta);
		}
    }
}

static u64 __search_rm_lx_page_table(u64 vfn, u8 lev, u64* pt_va, page_table_release ptr)
{
	u16 count;
	if (lev == 3) {
		u16 l3idx = get_levelx_idx(vfn, 3);
		//vfn is not mapped, error!
		if (PTI_PTYPE_IS_INV == get_level3_type(pt_va[l3idx])) {
            return MAP_STATUS_NOTMAPPED;
        } else {
			set_pti_type(pt_va[l3idx], PTI_PTYPE_IS_INV);
			//if current page table empty, release table page
			count = __lev3_self_minus(pt_va);
			if (!count)
				ptr(vfn<<PAGE_SHIFT);

            return 0;
        }
	} else {
		u16 lxidx = get_levelx_idx(vfn, lev);
        if (L02_NEXT_LEVEL_IS_INV == get_level02_type(pt_va[lxidx])) {
			return MAP_STATUS_NOTMAPPED;
		} else {
			u64 next_lev_pt_va = get_nextl_pt_linear_va(pt_va[lxidx]);
			if (MAP_STATUS_NOTMAPPED == __search_rm_lx_page_table(vfn, lev + 1, (u64*)next_lev_pt_va, ptr)) {
				return MAP_STATUS_NOTMAPPED;
			} else {
				set_pti_type(pt_va[lxidx], PTI_NEXTL_IS_INV);
				count = __lev02_self_minus(pt_va);
				if (!count)
					ptr(vfn<<PAGE_SHIFT);
            	return 0;
			}
		}
	}
}

/**
 * @description: 
 * @param {u64} vfn
 * @param {u8} lev
 * @param {u64*} pt_va
 * @return {*} pfn of page
 */
static u64 __search_lx_page_table(u64 vfn, u8 lev, u64* pt_va)
{
	u16 count;
	if (lev == 3) {
		u16 l3idx = get_levelx_idx(vfn, 3);
		//vfn is not mapped, error!
		if (PTI_PTYPE_IS_INV == get_level3_type(pt_va[l3idx])) {
            return INVALID_PFN;
        } else {
			return get_level3_pfn(pt_va[l3idx]);
        }
	} else {
		u16 lxidx = get_levelx_idx(vfn, lev);
        if (L02_NEXT_LEVEL_IS_INV == get_level02_type(pt_va[lxidx])) {
			return INVALID_PFN;
		} else {
			u64 next_lev_pt_va = get_nextl_pt_linear_va(pt_va[lxidx]);
			return __search_lx_page_table(vfn, lev + 1, (u64*)next_lev_pt_va);
		}
	}
}

u64 map_pages(u64 pfn, u64 vfn, u64 pages, u64 attr, u64* pt_root_va, page_table_alloc pta)
{
	u64 ret = 0;
	while (pages) {
		u16 l1idx = get_levelx_idx(vfn, 0);
		if (L02_NEXT_LEVEL_IS_INV != get_level02_type(pt_root_va[l1idx])) {
			u64 l1pt = get_nextl_pt_linear_va(pt_root_va[l1idx]);
			ret |= __search_add_lx_page_table(pfn, vfn, 1, attr, (u64*)l1pt, pta);
			if (ret & MAP_STATUS_ABORT)
				return ret;
		} else {
			u64* next_lev_pt = pta();
			if (!next_lev_pt)
				return MAP_STATUS_ABORT;
			__page_table_clr(next_lev_pt);
			pt_root_va[l1idx] = L02_NEXT_LEVEL_IS_PTI(get_pa_by_linear_va((u64)next_lev_pt)>>PAGE_SHIFT);
			ret |= __search_add_lx_page_table(pfn, vfn, 1, attr, next_lev_pt, pta);
		}
		pfn++; vfn++; pages--;
	}
	return ret;
}

u64 unmap_pages(u64 vfn, u64 pages, u64* pt_root_va, page_table_release ptr) 
{
	u64 ret = 0;
	while (pages) {
		ret |= __search_rm_lx_page_table(vfn, 0, pt_root_va, ptr);
		vfn++; pages++;
	}
	return ret;
}

u64 mapped_pfn(u64 vfn, u64* pt_root_va)
{
	return __search_lx_page_table(vfn, 0, pt_root_va);
}
