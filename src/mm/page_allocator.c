#include "mm/page_allocator.h"
#include "tools/helper.h"
#include "printf.h"

extern unsigned long kernel_phy_s;
extern unsigned long kernel_phy_e;
extern unsigned long early_page_table_phy_s;
extern unsigned long early_page_table_phy_e;
extern unsigned long early_stack_phy_s;
extern unsigned long early_stack_phy_e;
extern unsigned long page_item_phy_s;
extern unsigned long page_item_phy_e;
extern unsigned long early_heap_phy_s;
extern unsigned long early_heap_phy_e;

#define get_linear_va_by_page(base,p)           (KERN_LINEAR_REGION_S + ((((u64)(p)-(u64)(base))/sizeof(struct page))<<PAGE_SHIFT) + SYS_INFO_MEM_START)
#define get_pa_by_page(base,p)					(((((u64)(p)-(u64)(base))/sizeof(struct page))<<PAGE_SHIFT) + SYS_INFO_MEM_START)
#define get_prev_pidx(p)						((p)->attr0_order_prevpidx & ((1<<PAGE_ORDER_SHIFT)-1))
#define get_next_pidx(p)						((p)->attr1_subs_nextpidx & ((1<<PAGE_SUBS_SHIFT)-1))
#define get_attr(p)								((((p)->attr0_order_prevpidx>>PAGE_ATTR0_SHIFT)&0xff) | ((((p)->attr1_subs_nextpidx>>PAGE_ATTR1_SHIFT)&0x3f)<<8))
#define get_order(p)							(((p)->attr0_order_prevpidx>>PAGE_ORDER_SHIFT)&0xf)
#define get_subs(p)								(((p)->attr1_subs_nextpidx>>PAGE_SUBS_SHIFT)&0x3f)
#define get_page_by_p_idx(base,idx)				((void*)((u64)(base) + sizeof(struct page) * (idx)))
#define get_p_idx_by_page(base,p)				(((u64)(p) - (u64)(base))/sizeof(struct page))
#define get_page_by_pfn(base,pfn)				(get_page_by_p_idx(base,pfn_to_p_idx(pfn)))
#define get_prev_page(base,p)					((void*)((u64)(base) + sizeof(struct page)*get_prev_pidx(p)))
#define get_next_page(base,p)					((void*)((u64)(base) + sizeof(struct page)*get_next_pidx(p)))

#define set_prev_pidx(p,idx)					do{ \
													(p)->attr0_order_prevpidx &= (0xffffffffffffffffUL<<PAGE_ORDER_SHIFT); \
													(p)->attr0_order_prevpidx |= (((idx)&(1UL<<PAGE_ORDER_SHIFT)-1)); \
												}while(0)

#define set_next_pidx(p,idx)					do{ \
													(p)->attr1_subs_nextpidx &= (0xffffffffffffffffUL<<PAGE_SUBS_SHIFT); \
													(p)->attr1_subs_nextpidx |= (((idx)&(1UL<<PAGE_ORDER_SHIFT)-1)); \
												}while(0)

#define set_attr(p,attr)						do{ \
													(p)->attr0_order_prevpidx &= ~(0xffUL<<PAGE_ATTR0_SHIFT); \
													(p)->attr1_subs_nextpidx &= ~(0x3fUL<<PAGE_ATTR1_SHIFT); \
													(p)->attr0_order_prevpidx |= ((attr) & 0xffUL)<<PAGE_ATTR0_SHIFT; \
													(p)->attr1_subs_nextpidx |= (((attr)>>8)&0x3fUL)<<PAGE_ATTR1_SHIFT; \
												}while(0)
												
#define set_order(p,order)						do{ \
													(p)->attr0_order_prevpidx &= ~(0xfUL<<PAGE_ORDER_SHIFT); \
													(p)->attr0_order_prevpidx |= ((order)&0xfUL)<<PAGE_ORDER_SHIFT; \
												}while(0)

#define set_subs(p,subs)						do{ \
													(p)->attr1_subs_nextpidx &= ~(0x3fUL<<PAGE_SUBS_SHIFT); \
													(p)->attr1_subs_nextpidx |= ((subs)&0x3fUL)<<PAGE_SUBS_SHIFT; \
												}while(0)

#define set_prev_page(base,p,prev_p)			do{ \
													(p)->attr0_order_prevpidx &= (0xffffffffffffffffUL<<PAGE_ORDER_SHIFT); \
													(p)->attr0_order_prevpidx |= ((u64)(prev_p)-(u64)(base))/sizeof(struct page); \
												}while(0)

#define set_next_page(base,p,next_p)			do{ \
													(p)->attr1_subs_nextpidx &= (0xffffffffffffffffUL<<PAGE_SUBS_SHIFT); \
													(p)->attr1_subs_nextpidx |= ((u64)(next_p)-(u64)(base))/sizeof(struct page); \
												}while(0)

#define clr_page(p)								do {	\
													(p)->attr1_subs_nextpidx = 0;	\
													(p)->attr0_order_prevpidx = 0;	\
												}while(0)

struct page*                g_page = NULL;
struct page_area            g_page_area;

static struct page* __add_to_page_list_tail(struct page* head, struct page* page)
{
    if (!head || !page || head == page)
        return NULL;
	struct page_order* po = container_of(head, struct page_order, list_head);
    if (get_attr(head) & PAGE_ATTR_LIST_LAST) {
        set_prev_page(g_page, head, page);
        set_next_page(g_page, head, page);
        u64 attr = get_attr(head);
        attr &= ~PAGE_ATTR_LIST_LAST;
        set_attr(head, attr);
        attr = get_attr(page);
        attr |= PAGE_ATTR_LIST_LAST;
        set_attr(page, attr);
    } else {
        struct page* last = get_prev_page(g_page, head);
        set_prev_page(g_page, head, page);
        set_next_page(g_page, last, page);
        set_prev_page(g_page, page, last);
        u64 attr = get_attr(last);
        attr &= ~PAGE_ATTR_LIST_LAST;
        set_attr(last, attr);
        attr = get_attr(page);
        attr |= PAGE_ATTR_LIST_LAST;
        set_attr(page, attr);
    }
	po->counter++;
    return page;
}

static struct page* __add_to_page_list_head(struct page* head, struct page* page)
{
    if (!head || !page || head == page)
        return NULL;
	struct page_order* po = container_of(head, struct page_order, list_head);
    if (get_attr(head) & PAGE_ATTR_LIST_LAST) {
    	set_prev_page(g_page, head, page);
        set_next_page(g_page, head, page);
        u64 attr = get_attr(head);
        attr &= ~PAGE_ATTR_LIST_LAST;
        set_attr(head, attr);
        attr = get_attr(page);
        attr |= PAGE_ATTR_LIST_LAST;
        set_attr(page, attr);
    } else {
        struct page* first = get_next_page(g_page, head);
        set_next_page(g_page, head, page);
        set_next_page(g_page, page, first);
        set_prev_page(g_page, first, page);
    }
	po->counter++;
    return page;
}

static struct page* __insert_to_page_list(struct page* head, struct page* prev, struct page* page)
{
    if (!head || !prev || !page || head == prev || head == page || prev == page)
        return NULL;
	struct page_order* po = container_of(head, struct page_order, list_head);
    if (get_attr(head) & PAGE_ATTR_LIST_LAST)
        return NULL;
    if (get_attr(prev) & PAGE_ATTR_LIST_LAST) {
        set_prev_page(g_page, head, page);
        set_next_page(g_page, prev, page);
        set_prev_page(g_page, page, prev);
        u64 attr = get_attr(prev);
        attr &= ~PAGE_ATTR_LIST_LAST;
        set_attr(prev, attr);
        attr = get_attr(page);
        attr |= PAGE_ATTR_LIST_LAST;
        set_attr(page, attr);
    } else {
        struct page* next = get_next_page(g_page, prev);
        set_next_page(g_page, prev, page);
        set_prev_page(g_page, page, prev);
        set_next_page(g_page, page, next);
        set_prev_page(g_page, next, page);
    }
	po->counter++;
    return page;
}

static struct page* __remove_from_page_list(struct page* head, struct page* page)
{
    if (!head || !page || head == page)
        return NULL;
	struct page_order* po = container_of(head, struct page_order, list_head);
    u64 attr;
    struct page* prev,*next;
    prev = get_prev_page(g_page, head);
    next = get_next_page(g_page, head);
    if (prev == page && next == page) {                 //list only 1 item is page
        attr = get_attr(head);
        attr |= PAGE_ATTR_LIST_LAST;
        set_attr(head, attr);
    } else if (prev == page) {                          //page is last item but not first one
        prev = get_prev_page(g_page, page);
        attr = get_attr(prev);
        attr |= PAGE_ATTR_LIST_LAST;
        set_attr(prev, attr);
        attr = get_attr(page);
        attr &= ~PAGE_ATTR_LIST_LAST;
        set_attr(page, attr);
    } else if (next == page) {                          //page is first item but not last one
        next = get_next_page(g_page, page);
        set_next_page(g_page, head, next);
    } else {                                            //page nither first nor last
        prev = get_prev_page(g_page, page);
        next = get_next_page(g_page, page);
        set_next_page(g_page, prev, next);
        set_prev_page(g_page, next, prev);
    }
	po->counter--;
    return page;
}

static u8 __page_list_empty(struct page* head)
{
	return get_attr(head) & PAGE_ATTR_LIST_LAST ? 1 : 0;
}

static u64 __get_partner_pidx(u64 pidx, u8 order)
{
    u64 pidx_part = 0;
    if (pidx%(1<<(order+1))) {
        pidx_part = pidx - pidx%(1<<(order+1));
    } else {
        pidx_part = pidx + (1<<order);
    }
    return pidx_part;
}

static struct page* __get_partner_page(struct page* page)
{
    u64 pidx = ((u64)page - (u64)g_page)/sizeof(struct page);
    u8 order = get_order(page);
    u64 pidx_part = __get_partner_pidx(pidx, order);
    return (struct page*)(pidx_part + (u64)g_page);
}

static u64 __get_groupheader_pidx(u64 pidx, u8 order)
{
	pidx &= (0xffffffffffffffffUL << order);
}

static void __split_from_higher_order(u64 pidx, u8 order, struct page_type* pt)
{
	struct page* page = get_page_by_p_idx(g_page, pidx);
	struct page* partnerpage;
	struct page* tmpp;
	u64 partner;
	u32 attr = get_attr(page);
	u16 tmp;
	//split from higher and higher order
	if (!(attr & PAGE_ATTR_ORDER_GROUP_HEADER) || !(get_order(page) == order))
		__split_from_higher_order(__get_groupheader_pidx(pidx, order + 1), order + 1, pt);
	//split from current order
	__remove_from_page_list(&pt->orders[order].list_head, get_page_by_p_idx(g_page, pidx));
	tmp = 0;
	while (tmp < (1<<(order - 1))) {
		tmpp = get_page_by_p_idx(g_page, pidx + tmp);
		//group header
		if (tmp == 0) {
			//clear all attr, only save group header
			attr = PAGE_ATTR_ORDER_GROUP_HEADER;
			set_attr(tmpp, attr);
		} else {
			//clear group header, because all page item not be init, avoid error
			attr = get_attr(tmpp);
			attr &= ~PAGE_ATTR_ORDER_GROUP_HEADER;
			set_attr(tmpp, attr);
		}
		set_order(tmpp, order - 1);
		tmp++;
	}
	__add_to_page_list_tail(&pt->orders[order-1].list_head, get_page_by_p_idx(g_page, pidx));
	partner = __get_partner_pidx(pidx, order - 1);
	tmp = 0;
	while (tmp < (1<<(order - 1))) {
		tmpp = get_page_by_p_idx(g_page, partner + tmp);
		//group header
		if (tmp == 0) {
			//clear all attr, only save group header
			attr = PAGE_ATTR_ORDER_GROUP_HEADER;
			set_attr(tmpp, attr);
		} else {
			//clear group header, because all page item not be init, avoid error
			attr = get_attr(tmpp);
			attr &= ~PAGE_ATTR_ORDER_GROUP_HEADER;
			set_attr(tmpp, attr);
		}
		set_order(tmpp, order - 1);
		tmp++;
	}
	__add_to_page_list_tail(&pt->orders[order-1].list_head, get_page_by_p_idx(g_page, partner));
}

static void __merge_to_higher_order(u64 pidx, u8 order, struct page_type* pt)
{
	struct page* tmpp, *p = get_page_by_p_idx(g_page, pidx);
	struct page* partner_page = __get_partner_page(p);
	u64 pidx_header, tmp;
	u32 attr, part_attr = get_attr(partner_page);
	u8 part_order = get_order(partner_page);
	if ((part_attr & PAGE_ATTR_ORDER_GROUP_HEADER) && (!(part_attr & PAGE_ATTR_USED)) && (part_order == order)) {
		__remove_from_page_list(&pt->orders[order].list_head, p);
		__remove_from_page_list(&pt->orders[order].list_head, partner_page);
		pidx_header = __get_groupheader_pidx(pidx, order + 1);
		for (tmp = 0; tmp < (1<<(order + 1)); tmp++) {
			tmpp = get_page_by_p_idx(g_page, pidx_header + tmp);
			attr = get_attr(tmpp);
			if (tmp == 0) {
				attr |= PAGE_ATTR_ORDER_GROUP_HEADER;
			} else {
				attr &= ~PAGE_ATTR_ORDER_GROUP_HEADER;
			}
			set_attr(tmpp, attr);
			set_order(tmpp, order + 1);
		}
		__add_to_page_list_tail(&pt->orders[order+1].list_head, get_page_by_p_idx(g_page, pidx_header));
		//try to merge higher
		if (order + 1 == MAX_ORDER)
			return;

		__merge_to_higher_order(pidx_header, order + 1, pt);
	}
}

static u8 __find_max_lower_order(u64 pidx, u64 pages)
{
	u8 order = MAX_ORDER;
	while (order >= 0) {
		if (!(pidx % (1<<order)))
			if (pages >= (1<<order))
				break;
		order--;
	}
	return order;
}

static struct page* __alloc_pages(u8 order)
{
	struct page* page;
	//get pages from current order list
	if (!__page_list_empty(&g_page_area.unused.orders[order].list_head)) {
		page = get_page_by_p_idx(g_page, get_next_pidx(&g_page_area.unused.orders[order].list_head));
		__remove_from_page_list(&g_page_area.unused.orders[order].list_head, page);
		return page;
	}
	//get pages from higher level order list, any higher order list first item
	u8 found = 0;
	u8 ho = order + 1;
	while (ho <= MAX_ORDER) {
		if (__page_list_empty(&g_page_area.unused.orders[ho].list_head)) {
			ho++;
			continue;
		}
		found = 1;
		break;
	}
	if (!found) {
		early_dbg_print(PRT_ERROR, "no page area satisfied\r\n");
		return NULL;
	}
	page = get_page_by_p_idx(g_page, get_next_pidx(&g_page_area.unused.orders[ho].list_head));
	u64 pidx = get_p_idx_by_page(g_page, page);
	//pidx is higher order group header, it is must be a group header, not need to calculate group header
	__split_from_higher_order(pidx, order + 1, &g_page_area.unused);
	page = get_page_by_p_idx(g_page, get_next_pidx(&g_page_area.unused.orders[order].list_head));
	__remove_from_page_list(&g_page_area.unused.orders[order].list_head, page);
	return page;
}

static struct page* __alloc_page()
{
	return __alloc_pages(0);
}

static struct page* __alloc_fragment_pages(u64 num, u8* porder)
{
	struct page* p = NULL;
	u8 order = 0;
	//found fragment pages
	while ((1UL<<order) <= num) {
		if (!__page_list_empty(&g_page_area.unused.orders[order].list_head)) {
			p = get_page_by_p_idx(g_page, get_next_pidx(&g_page_area.unused.orders[order].list_head));
			__remove_from_page_list(&g_page_area.unused.orders[order].list_head, p);
			*porder = order;
			return p;
		} else {
			order++;
		}
	}
	//have no fragment pages, split from higher order
	*porder = order -1;
	return __alloc_pages(order - 1);
}

static void __alloc_pages_to_subsystem(u64 pidx, u8 order, enum SUB_SYSTEM subs)
{
	u32 attr;
	struct page* pagetmp;
	u64 pages = (1<<order);
	u64 pidxtmp = pidx, tmp = 0;
	while (tmp < pages) {
		pagetmp = get_page_by_p_idx(g_page, pidxtmp);
		if (tmp != 0) {
			attr = PAGE_ATTR_USED;
		} else {
			attr = get_attr(pagetmp);
			attr |= PAGE_ATTR_USED;
		}
		set_attr(pagetmp, attr);
		set_subs(pagetmp, subs);
		pidxtmp++;
		tmp++;
	}
	pagetmp = get_page_by_p_idx(g_page, pidx);
	__add_to_page_list_tail(&g_page_area.used[subs].orders[order].list_head, pagetmp);
}

static void __set_pages_used(u64 pidx, u8 order, u8 used)
{
	u64 tmp;
	struct page* p;
	u32 attr;
	for (tmp = 0; tmp < (1UL<<order); tmp++) {
		p = get_page_by_p_idx(g_page, pidx + tmp);
		attr = get_attr(p);
		attr &= ~PAGE_ATTR_USED;
		attr |= used?PAGE_ATTR_USED:0;
	}
}

static s8 __dealloc_pages_from_subsystem(u64 pidx, u8 order)
{
	struct page* p = get_page_by_p_idx(g_page, pidx);
	u8 subs = get_subs(p);
	u32 attr = get_attr(p);
	u8 po = get_order(p);

	//unalloced page must not be release
	if (!(attr & PAGE_ATTR_USED))
		return -1;
	//simplest situation
	if ((po == order) && (attr & PAGE_ATTR_ORDER_GROUP_HEADER)) {
		__remove_from_page_list(&g_page_area.used[subs].orders[order].list_head, p);
		__set_pages_used(pidx, order, 0);
	} else if (po > order) {		//current order group bigger than required order, split from higher order to release part 
		//if pidx is not groupheader of required, parameter must be error
		if (pidx != __get_groupheader_pidx(pidx, order))
			return -1;
		__split_from_higher_order(__get_groupheader_pidx(pidx, order + 1), order + 1, &g_page_area.used[subs]);
		__remove_from_page_list(&g_page_area.used[subs].orders[order].list_head, p);
		__set_pages_used(pidx, order, 0);
	} else {						//current order group smaller than required
		return -1;
	}
	return 0;
}

static s8 __release_pages(u64 pidx, u8 order)
{
	struct page* partner_page;
	struct page* p = get_page_by_p_idx(g_page, pidx);

	if (__dealloc_pages_from_subsystem(pidx, order) < 0)
		return -1;
	//add released pages to unused list
	__add_to_page_list_tail(&g_page_area.unused.orders[order].list_head, p);
	//try to merge partner
	if (order < MAX_ORDER)
		__merge_to_higher_order(pidx, order, &g_page_area.unused);

	return 0;
}

static s8 __release_page(u64 pidx)
{
	return __release_pages(pidx, 0);
}

static struct page* __early_alloc_pages_from_pidx(u64 pidx, u8 order, enum SUB_SYSTEM subs)
{
	struct page* page = get_page_by_p_idx(g_page, pidx);
	u32 attr = get_attr(page);
	if (attr & PAGE_ATTR_USED)
		return NULL;
	if (pidx%(1<<order))
		return NULL;
	if (!(attr & PAGE_ATTR_ORDER_GROUP_HEADER) || !(get_order(page) == order))
		__split_from_higher_order(pidx, order + 1, &g_page_area.unused);
	
	__remove_from_page_list(&g_page_area.unused.orders[order].list_head, page);
	__alloc_pages_to_subsystem(pidx, order, subs);
	return page;
}

static void __early_init_page_items()
{
	//add all pages to unused type
    u8  order = 0;
    u64 page_num = SYS_INFO_MEM_SIZE >> PAGE_SHIFT;
    u64 pidx = 0;
    struct page* page;
    //page search
    while ( page_num > 0 ) {
		order = __find_max_lower_order(pidx, page_num);
		page = get_page_by_p_idx(g_page, pidx);
		clr_page(page);
        set_order(page, order);
        u32 page_attr = get_attr(page);
        set_attr(page, page_attr | PAGE_ATTR_ORDER_GROUP_HEADER);
        __add_to_page_list_tail(&g_page_area.unused.orders[order].list_head, page);
		page_num -= (1<<order);
		pidx += (1<<order);
    }
	//print
	u8 tmp = 0;
	early_dbg_print(PRT_DEBUG, "unused managed pages:");
	while (tmp <= MAX_ORDER) {
		early_dbg_print(PRT_DEBUG, "order%d %d", tmp, g_page_area.unused.orders[tmp].counter);
		if (tmp == MAX_ORDER) {
			early_dbg_print(PRT_DEBUG, "\r\n");
		} else {
			early_dbg_print(PRT_DEBUG, ",");
		}
		tmp++;
	}
}

static struct page* __early_alloc_area_from_pidx(u64 phy_s, u64 phy_e, enum SUB_SYSTEM subs)
{
	u64 pidx, pages;
	u8 order;
	pidx = pfn_to_p_idx(phy_s>>PAGE_SHIFT);
	pages = (phy_e>>PAGE_SHIFT) - (phy_s>>PAGE_SHIFT);
	while (pages > 0) {
		order = __find_max_lower_order(pidx, pages);
		if (!__early_alloc_pages_from_pidx(pidx, order, subs)) {
			early_dbg_print(PRT_ERROR, "alloc kernel context page failed!\r\n");
			return NULL;
		}
		pidx += (1 << order);
		pages -= (1 << order);
	}
	return get_page_by_p_idx(g_page, pfn_to_p_idx(phy_s>>PAGE_SHIFT));
}

static void __early_reserve_used_pages()
{
	if (!__early_alloc_area_from_pidx(kernel_phy_s, kernel_phy_e, SUBS_KERN))
		return;
	if (!__early_alloc_area_from_pidx(early_stack_phy_s, early_stack_phy_e, SUBS_STACK))
		return;
	if (!__early_alloc_area_from_pidx(page_item_phy_s, page_item_phy_e, SUBS_PAGE_ALLOC))
		return;
	if (!__early_alloc_area_from_pidx(early_page_table_phy_s, early_page_table_phy_e, SUBS_PAGE_TABLE))
		return;
	//print
	u8 tmp = 0;
	early_dbg_print(PRT_DEBUG, "unused managed pages after reserve areas:");
	while (tmp <= MAX_ORDER) {
		early_dbg_print(PRT_DEBUG, "order%d %d", tmp, g_page_area.unused.orders[tmp].counter);
		if (tmp == MAX_ORDER) {
			early_dbg_print(PRT_DEBUG, "\r\n");
		} else {
			early_dbg_print(PRT_DEBUG, ",");
		}
		tmp++;
	}
	tmp = 0;
	early_dbg_print(PRT_DEBUG, "kernel subsystem managed pages:");
	while (tmp <= MAX_ORDER) {
		early_dbg_print(PRT_DEBUG, "order%d %d", tmp, g_page_area.used[SUBS_KERN].orders[tmp].counter);
		if (tmp == MAX_ORDER) {
			early_dbg_print(PRT_DEBUG, "\r\n");
		} else {
			early_dbg_print(PRT_DEBUG, ",");
		}
		tmp++;
	}
	tmp = 0;
	early_dbg_print(PRT_DEBUG, "stack managed pages:");
	while (tmp <= MAX_ORDER) {
		early_dbg_print(PRT_DEBUG, "order%d %d", tmp, g_page_area.used[SUBS_STACK].orders[tmp].counter);
		if (tmp == MAX_ORDER) {
			early_dbg_print(PRT_DEBUG, "\r\n");
		} else {
			early_dbg_print(PRT_DEBUG, ",");
		}
		tmp++;
	}
	tmp = 0;
	early_dbg_print(PRT_DEBUG, "page item managed pages:");
	while (tmp <= MAX_ORDER) {
		early_dbg_print(PRT_DEBUG, "order%d %d", tmp, g_page_area.used[SUBS_PAGE_ALLOC].orders[tmp].counter);
		if (tmp == MAX_ORDER) {
			early_dbg_print(PRT_DEBUG, "\r\n");
		} else {
			early_dbg_print(PRT_DEBUG, ",");
		}
		tmp++;
	}
	tmp = 0;
	early_dbg_print(PRT_DEBUG, "page table managed pages:");
	while (tmp <= MAX_ORDER) {
		early_dbg_print(PRT_DEBUG, "order%d %d", tmp, g_page_area.used[SUBS_PAGE_TABLE].orders[tmp].counter);
		if (tmp == MAX_ORDER) {
			early_dbg_print(PRT_DEBUG, "\r\n");
		} else {
			early_dbg_print(PRT_DEBUG, ",");
		}
		tmp++;
	}
}

static u64* __page_table_alloc()
{
	struct page* p = NULL;
	p = __alloc_page();
	if (!p)
		return NULL;
	__alloc_pages_to_subsystem(get_p_idx_by_page(g_page,p), 0, SUBS_PAGE_TABLE);
	return (u64*)get_linear_va_by_page(g_page, p);
}

static void __page_table_release(u64 linear_va)
{
	u64 pidx = pfn_to_p_idx(get_pa_by_linear_va(linear_va) >> PAGE_SHIFT);
	if (__release_page(pidx) < 0) {
		early_dbg_print(PRT_ERROR, "release page table page failed!\r\n");
	}
}

int init_page_allocator(struct page* pageitembase)
{
    u64 tmp = 0, tmp1 = 0;
    g_page = pageitembase;
    //clear list head
    for (tmp = 0; tmp < MAX_ORDER + 1; tmp++) {
        for (tmp1 = 0; tmp1 < SUBS_MAX; tmp1++)
        {
            g_page_area.used[tmp1].orders[tmp].list_head.attr0_order_prevpidx = 0;
            g_page_area.used[tmp1].orders[tmp].list_head.attr1_subs_nextpidx = 0;
			g_page_area.used[tmp1].orders[tmp].counter = 0;
            set_attr(&g_page_area.used[tmp1].orders[tmp].list_head, PAGE_ATTR_LIST_HEAD|PAGE_ATTR_LIST_LAST);
        }
        g_page_area.unused.orders[tmp].list_head.attr0_order_prevpidx = 0;
        g_page_area.unused.orders[tmp].list_head.attr1_subs_nextpidx = 0;
		g_page_area.unused.orders[tmp].counter = 0;
        set_attr(&g_page_area.unused.orders[tmp].list_head, PAGE_ATTR_LIST_HEAD|PAGE_ATTR_LIST_LAST);
    }
    //init all page items
	__early_init_page_items();
    //reserve used pages
	__early_reserve_used_pages();
	
	return 0;
}

struct page* alloc_pages(u8 order, enum page_map_type type, u64 va, enum SUB_SYSTEM subs) 
{
	struct page* p;
	u64 mapret;
	if (!(p = __alloc_pages(order))) 
		return NULL;
	__alloc_pages_to_subsystem(get_p_idx_by_page(g_page,p), order, subs);

	if (type == MAP_NOT && type == MAP_DIRECT)
		return p;
	
	if (type == MAP_ASSIGNED_VA) {
		mapret = map_pages(get_pa_by_page(g_page, p)>>PAGE_SHIFT, va>>PAGE_SHIFT, 1<<order, 
					L3_PFN_ATTR_RWX, (u64*)get_linear_va_by_pa((u64)g_kernel_pt_root), __page_table_alloc);
	}

	return p;
}

void* alloc_pages_discrete(u64 pages, u64 va, enum SUB_SYSTEM subs)
{
	u8 order = 0;
	u64 mapret;
	struct page* p = NULL;
	u64* retaddr = (u64*)va;
	while (pages) {
		p = __alloc_fragment_pages(pages, &order);
		if (!p)
			return NULL;

		__alloc_pages_to_subsystem(get_p_idx_by_page(g_page,p), order, subs);

		mapret = map_pages(get_pa_by_page(g_page, p)>>PAGE_SHIFT, va>>PAGE_SHIFT, 1<<order,
					L3_PFN_ATTR_RWX, (u64*)get_linear_va_by_pa((u64)g_kernel_pt_root), __page_table_alloc);
		va += PAGE_SIZE;
		pages -= (1<<order);
	}
	return retaddr;
}

/**
 * @description: va and pa are all continuous
 * @param {u64} va (some subsystem can only get linear va, others have it own map. we need ummap subsystem's map)
 * @param {u64} order
 * @return {*}
 */
void release_pages(u64 va, u64 order)
{
	u64 pidx, pfn;
	if (va_in_linear_area(va)) {
		pidx = pfn_to_p_idx(get_pa_by_linear_va(va) >> PAGE_SHIFT);
	} else {
		pfn = mapped_pfn(va >> PAGE_SHIFT, (u64*)get_linear_va_by_pa((u64)g_kernel_pt_root));
		if (pfn == INVALID_PFN) {
			early_dbg_print(PRT_ERROR, "release ordered page failed! can't get pfn\r\n");
			return;
		}
		pidx = pfn_to_p_idx(pfn);
	}
	if (__release_pages(pidx, order) < 0) {
		early_dbg_print(PRT_ERROR, "release ordered page failed!\r\n");
		return;
	}
	//released by linear va, not need be remove from page table
	if (!va_in_linear_area(va)) {
		unmap_pages(va >> PAGE_SHIFT, 1UL << order, (u64*)get_linear_va_by_pa((u64)g_kernel_pt_root), __page_table_release);
	}
}

void release_pages_discrete(u64 va, u64 pages)
{
	u64 pfn;
	while (pages) {
		release_pages(va, 0);
		va++; pages--;
	}
}