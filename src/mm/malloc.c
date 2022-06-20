#include "mm/malloc.h"

struct malloc_block_base*   g_base;

static void block_header_zero(struct malloc_block_header* header)
{
    rbnode_zero(&header->addr_tree_node);
    rbnode_zero(&header->gap_tree_node);
    rbnode_zero(&header->buff_tree_node);
    rbnode_zero(&header->nonmgtva_tree_node);
    header->buff_list.p_next = NULL;
    header->buff_list.p_prev = NULL;
    header->pos_list.p_next = NULL;
    header->pos_list.p_prev = NULL;
    header->non_mgt_va_page = 0;
    header->mgt_max_va = 0;
    header->attr = 0;
}

static void block_base_zero(struct malloc_block_base* base)
{
    u8 tmp;
    base->gap_root.node = NULL;
    base->addr_root.node = NULL;
    base->buff_root.node = NULL;
    base->nonmgtva_root.node = NULL;
    base->pos_head.p_next = NULL;
    base->pos_head.p_prev = NULL;
}

static u64 get_gap_by_header(struct malloc_block_header* header)
{
    struct malloc_block_header* next_h;
    u64 gap = 0;
    if (mbh_get_status(header) & ATTR_VA_LAST)
        gap = header->mgt_max_va - (u64)header - mbh_get_extra_len(header) - mbh_get_data_buff_len(header);
    else {
        next_h = container_of(header->pos_list.p_next, struct malloc_block_header, pos_list);
        gap = (u64)next_h - (u64)header - mbh_get_extra_len(header)  - mbh_get_data_buff_len(header);
    }
    if ((mbh_get_status(header) & ATTR_VA_FIRST) && (mbh_get_status(header) & ATTR_NOT_USED)) {
        gap += sizeof(struct malloc_block_header);
    }
    return gap;
}

static s8 gap_rbtree_node_instert_cmp(Node* a,Node* b)
{
    struct malloc_block_header* ha,*hb;
    u64 gapa,gapb;
    ha = container_of(a,struct malloc_block_header,gap_tree_node);
    hb = container_of(b,struct malloc_block_header,gap_tree_node);
    gapa = get_gap_by_header(ha);
    gapb = get_gap_by_header(hb);
    return gapa > gapb ? 1 : (gapa == gapb ? 0 : -1);
}

static s8 addr_rbtree_node_insert_cmp(Node* a,Node* b)
{
    struct malloc_block_header* ha,*hb;
    u64 addra,addrb;
    ha = container_of(a,struct malloc_block_header,addr_tree_node);
    hb = container_of(b,struct malloc_block_header,addr_tree_node);
    addra = (u64)ha + mbh_get_extra_len(ha);
    addrb = (u64)hb + mbh_get_extra_len(hb);
    return addra > addrb ? 1 : (addra == addrb ? 0 : -1);
}

static s8 buff_rbtree_node_insert_cmp(Node* a,Node* b)
{
    struct malloc_block_header* ha,*hb;
    u32 buffa,buffb;
    ha = container_of(a,struct malloc_block_header,buff_tree_node);
    hb = container_of(b,struct malloc_block_header,buff_tree_node);
    buffa = mbh_get_itemlen(ha);
    buffb = mbh_get_itemlen(hb);
    return buffa > buffb ? 1 : (buffa == buffb ? 0 : -1);
}

static s8 nonmgtva_tree_node_insert_cmp(Node* a, Node* b)
{
    struct malloc_block_header* ha,* hb;
    u64 page_a, page_b;
    ha = container_of(a, struct malloc_block_header, nonmgtva_tree_node);
    hb = container_of(b, struct malloc_block_header, nonmgtva_tree_node);
    page_a = ha->non_mgt_va_page;
    page_b = hb->non_mgt_va_page;
    return page_a > page_b ? 1 : (page_a == page_b ? 0 : -1);
}

static s8 buff_rbtree_node_search_cmp(Node* node,void* param)
{
    struct malloc_block_header* hnode;
    u32 ilennode, ilenp;
    hnode = container_of(node,struct malloc_block_header,buff_tree_node);
    ilennode = mbh_get_itemlen(hnode);
    ilenp = *(u32*)param;
    return ilennode > ilenp ? 1 : (ilennode == ilenp ? 0 : -1);
}

static s8 gap_rbtree_node_search_cmp(Node* node,void* param)
{
    struct malloc_block_header* hnode;
    u64 gapnode, gapp;
    hnode = container_of(node,struct malloc_block_header,gap_tree_node);
    gapnode = get_gap_by_header(hnode);
    gapp = *(u64*)param;
    return gapnode > gapp ? 1 : (gapnode == gapp ? 0 : -1);
}

static s8 addr_rbtree_node_search_cmp(Node* node, void* param)
{
    struct malloc_block_header* hnode;
    u64 addrn, addrp;
    hnode = container_of(node,struct malloc_block_header,addr_tree_node);
    addrn = mbh_get_data_p(hnode);
    addrp = *(u64*)param;
    return addrn > addrp ? 1 : (addrn == addrp ? 0 : -1);
}

struct malloc_block_base* init_malloc_sys(u64 va_start, u64 len)
{
    struct malloc_block_header* first_header;
    struct malloc_block_base* block_base;
    int tmp;
    u32 first_gap;
    if ((va_start & PAGE_MASK) || (len & PAGE_MASK))
        return NULL;
    if (len < sizeof(struct malloc_block_base) + mbh_get_extra_len_by_num(sizeof(struct malloc_block_base)))
        return NULL;

    first_header = (struct malloc_block_header*)va_start;
    block_base = (struct malloc_block_base*)(va_start + mbh_get_extra_len_by_num(sizeof(struct malloc_block_base)));
    g_base = block_base;
    block_header_zero(first_header);

    mbh_set_status(first_header, ATTR_VA_FIRST | ATTR_VA_LAST | ATTR_BUFF_FIRST | ATTR_NOT_CLEAR_IMED);
    mbh_set_itemnum(first_header, 1);
    mbh_set_itemlen(first_header, MEM_ROUND_UP(sizeof(struct malloc_block_base), 0x7));
    first_header->maps[0] = 0;
    mbh_set_idx_map(first_header, 0, 1);
    first_header->mgt_max_va = va_start + len;
    first_header->non_mgt_va_page = (MAX_VA - first_header->mgt_max_va) >> PAGE_SHIFT;

    block_base_zero(block_base);

    list_add_tail(&block_base->pos_head, &first_header->pos_list);
    insert_rbtree(&block_base->gap_root, &first_header->gap_tree_node, gap_rbtree_node_instert_cmp);
    insert_rbtree(&block_base->addr_root, &first_header->addr_tree_node, addr_rbtree_node_insert_cmp);
    insert_rbtree(&block_base->buff_root, &first_header->buff_tree_node, buff_rbtree_node_insert_cmp);
    insert_rbtree(&block_base->nonmgtva_root, &first_header->nonmgtva_tree_node, nonmgtva_tree_node_insert_cmp);

    return block_base;
}

static u64 get_suggest_buff_len(u32 itemlen, u16 layer)
{
    u64 ret = (u64)FIRST_SUGGEST_ITEMNUM(itemlen) << layer;
    return ret > MAX_SUGGEST_ITEMNUM(itemlen) ? MAX_SUGGEST_ITEMNUM(itemlen) : ret;
}

static u64 get_suggest_buff_len_by_header(struct malloc_block_header* header, u16 layer)
{
    return get_suggest_buff_len(mbh_get_itemlen(header), layer);
}

static u8 buff_full_chk(struct malloc_block_header* header)
{
    u64 itemnum, tmp=0;
    itemnum = mbh_get_itemnum(header);
    while (tmp < itemnum) {
        if (itemnum - tmp < 64) {
            u64 fullmask = ~(0xffffffffffffffffUL << (itemnum - tmp));
            if (fullmask == header->maps[tmp/64])
                break;
            else
                return 0;
        } else {
            if (header->maps[tmp/64] == 0xffffffffffffffffUL) {
                tmp += 64;
                continue;
            } else {
                return 0;
            }
        }
    }
    return 1;
}

static u8 buff_empty_chk(struct malloc_block_header* header)
{
    u64 itemnum, tmp=0;
    itemnum = mbh_get_itemnum(header);
    while (tmp < itemnum) {
        if (header->maps[tmp/64] == 0) {
            tmp += 64;
            continue;
        } else {
            return 0;
        }
    }
    return 1;
}

static void* alloc_from_this_buff(struct malloc_block_header* header)
{
    u16 status, itemnum, tmp=0;
    status = mbh_get_status(header);
    itemnum = mbh_get_itemnum(header);
    if (status & ATTR_FULL)
        return NULL;
    while (tmp < itemnum) {
        if (header->maps[tmp/64] == 0xffffffffffffffffUL) {
            tmp += 64;
            continue;
        }
        if (!mbh_get_idx_map(header, tmp))
            break;
        tmp++;
    }
    mbh_set_idx_map(header, tmp, 1);
    if (buff_full_chk(header))
        mbh_set_status(header, status | ATTR_FULL);
    return (void*)(mbh_get_data_p(header) + tmp * mbh_get_itemlen(header));
}

static void malloc_block_delete(struct malloc_block_header* header)
{
    if (mbh_get_status(header) & ATTR_VA_LAST) {
        struct malloc_block_header* preh = 
            container_of(header->pos_list.p_prev, struct malloc_block_header, pos_list);
        if (!(mbh_get_status(preh) & ATTR_VA_LAST)) {

        }
    }
}

static void dealloc_from_this_buff(struct malloc_block_header* header, u16 idx) 
{
    if (!mbh_get_idx_map(header, idx))                               //status error
        return;

    mbh_set_idx_map(header, idx, 0);

    if (!buff_empty_chk(header))
        return;
    
    if (mbh_get_status(header) & ATTR_NOT_CLEAR_IMED)               //not dealloc buff imediately
        return;

    if ((mbh_get_status(header) & ATTR_BUFF_FIRST) && !(list_empty(&header->buff_list)))    //buff first and list not empty
        return;

    if ((mbh_get_status(header) & ATTR_VA_LAST) && !(mbh_get_status(header) & ATTR_VA_FIRST)) {     //!S and E
        struct malloc_block_header* preh = 
            container_of(header->pos_list.p_prev, struct malloc_block_header, pos_list);
        u16 attr_prev = mbh_get_status(preh);
        attr_prev |= ATTR_VA_LAST;
        mbh_set_status(preh, attr_prev);
        preh->mgt_max_va = header->mgt_max_va;
        preh->non_mgt_va_page = header->non_mgt_va_page;
        delete_rbtree(&g_base->nonmgtva_root, &header->nonmgtva_tree_node);
        insert_rbtree(&g_base->nonmgtva_root, &preh->nonmgtva_tree_node, nonmgtva_tree_node_insert_cmp);
        //remove from pos list
        list_remove(&g_base->pos_head, &header->pos_list);
        //prev header gap will be changed after header dealloc
        delete_rbtree(&g_base->gap_root, &preh->gap_tree_node);
        insert_rbtree(&g_base->gap_root, &preh->gap_tree_node, gap_rbtree_node_instert_cmp);
        //header gap invalid, remove
        delete_rbtree(&g_base->gap_root, &header->gap_tree_node);
    } else if (mbh_get_status(header) & ATTR_VA_FIRST) {                                           //(S and E) or (S and !E)
        //attr set
        u16 attr = mbh_get_status(header);
        attr |= ATTR_NOT_USED;
        mbh_set_status(header, attr);
        //itemlen itemnum
        mbh_set_itemlen(header, 0);
        mbh_set_itemnum(header, 0);
        //gap will chage, but don't remove gap item
        delete_rbtree(&g_base->gap_root, &header->gap_tree_node);
        insert_rbtree(&g_base->gap_root, &header->gap_tree_node, gap_rbtree_node_instert_cmp);
    } else {                                                                                       //!S and !E
        struct malloc_block_header* preh = 
            container_of(header->pos_list.p_prev, struct malloc_block_header, pos_list);
        //remove from pos list
        list_remove(&g_base->pos_head, &header->pos_list);
        //prev header gap will be changed after header dealloc
        delete_rbtree(&g_base->gap_root, &preh->gap_tree_node);
        insert_rbtree(&g_base->gap_root, &preh->gap_tree_node, gap_rbtree_node_instert_cmp);
        //header gap invalid, remove
        delete_rbtree(&g_base->gap_root, &header->gap_tree_node);
    }
    //general operations
    if (mbh_get_status(header) & ATTR_BUFF_FIRST)                   //buff first need be remove
        delete_rbtree(&g_base->buff_root, &header->buff_tree_node);
    else
        list_remove(&header->buff_head->buff_list, &header->buff_list);
    delete_rbtree(&g_base->addr_root, &header->addr_tree_node);
    //if last header in va block, free page
    if ((mbh_get_status(header) & ATTR_VA_FIRST) && (mbh_get_status(header) & ATTR_VA_LAST))
        malloc_block_delete(header);
}

static void* alloc_from_old_buff(struct malloc_block_header* header, u32* layer) 
{
    void* ret = NULL;
    *layer = 0;
    if ((ret = alloc_from_this_buff(header)) != NULL)              //first layer hited
        return ret;
    if (list_empty(&header->buff_list))
        return NULL;
    struct malloc_block_header* mbh;
    list_for_each_entry(mbh, &header->buff_list, buff_list) {
        layer++;
        if ((ret = alloc_from_this_buff(mbh)) != NULL)
            return ret;
    }
    return NULL;
}

static void* alloc_from_new_buff(struct malloc_block_header* header, u32 itemlen, u32 layer) 
{
    u64 s_buf_count, s_mem_size;
    if (!header) {
        s_buf_count = get_suggest_buff_len(itemlen, layer);
    } else {
        s_buf_count = get_suggest_buff_len_by_header(header, layer);
    }
    Node* gapnode = NULL;
    while (s_buf_count > 0) {                               //memory maybe not enough for suggested buff, take half and try again
        s_mem_size = s_buf_count * itemlen + mbh_get_extra_len_by_num(s_buf_count);
        if (!rbtree_empty(&g_base->gap_root))
            gapnode = rbtree_smallest_bigger(&g_base->gap_root, &s_mem_size, gap_rbtree_node_search_cmp);
        if (gapnode)
            break;
        s_buf_count >>= 1;
        if (!s_buf_count)
            return NULL;
    }
    struct malloc_block_header* gaph = container_of(gapnode, struct malloc_block_header, gap_tree_node);
    struct malloc_block_header* newh = NULL;
    u16    attr=0;
    u8 va_first_empty = 0;
    if ((mbh_get_status(gaph) & ATTR_VA_FIRST) && (mbh_get_status(gaph) & ATTR_NOT_USED)) {             //va first item can be reused
        newh = gaph;
        va_first_empty = 1;
        //clear attr
        attr = mbh_get_status(newh);
        attr &= ~ATTR_NOT_USED;
        attr &= ~ATTR_FULL;
        mbh_set_status(newh, attr);
    } else {
        newh = (struct malloc_block_header*)mbh_get_gap_p(gaph);
        block_header_zero(newh);
        //set gap header attr and new header attr
        if (!header)
            attr |= ATTR_BUFF_FIRST;
        if (mbh_get_status(gaph) & ATTR_VA_LAST) {      //if gap header is VA_LAST new header will inherit it
            u16 attr_gap = mbh_get_status(gaph);
            attr_gap &= ~ATTR_VA_LAST;
            mbh_set_status(gaph, attr_gap);
            attr |= ATTR_VA_LAST;
            newh->non_mgt_va_page = gaph->non_mgt_va_page;
            newh->mgt_max_va = gaph->mgt_max_va;
            delete_rbtree(&g_base->nonmgtva_root, &gaph->nonmgtva_tree_node);
            insert_rbtree(&g_base->nonmgtva_root, &newh->nonmgtva_tree_node, nonmgtva_tree_node_insert_cmp);
        }
    }
    mbh_set_status(newh, attr);
    mbh_set_itemnum(newh, s_buf_count);
    mbh_set_itemlen(newh, itemlen);
    u16 tmp;
    mbh_clr_map(newh, tmp);
    mbh_set_idx_map(newh, 0, 1);
    //add new item to buff list tail and init attr_ext
    if (header) {
        list_add_tail(&header->buff_list, &newh->buff_list);
    } else {
        insert_rbtree(&g_base->buff_root, &newh->buff_tree_node, buff_rbtree_node_insert_cmp);
    }
    //add new item to position list
    list_insert(&g_base->pos_head, &gaph->pos_list, &newh->pos_list);
    //add new item to address search tree
    insert_rbtree(&g_base->addr_root, &newh->addr_tree_node, addr_rbtree_node_insert_cmp);
    //add new item to gap search tree and modify gap item
    delete_rbtree(&g_base->gap_root, &gaph->gap_tree_node);
    insert_rbtree(&g_base->gap_root, &gaph->gap_tree_node, gap_rbtree_node_instert_cmp);
    if (!va_first_empty) {
        insert_rbtree(&g_base->gap_root, &newh->gap_tree_node, gap_rbtree_node_instert_cmp);
    }
    return (void*)mbh_get_data_p(newh);
}

static u32 malloc_sys_request_pages(u32 pages)
{
    
}

void* malloc(u64 size)
{
    u32 tmp;
    void* addr = NULL;
    size = MEM_ROUND_UP(size, 0x7);                                                             //item was 8 bytes aligned, this is very important
    Node* buffnode = rbtree_search(&g_base->buff_root, &size, buff_rbtree_node_search_cmp);     //check already exist equal length buff
    if (buffnode) {
        struct malloc_block_header* header = container_of(buffnode, struct malloc_block_header, buff_tree_node);
        if ((addr = alloc_from_old_buff(header, &tmp)) != NULL) {                               //try old buffer
            return addr;
        } else {                                                                                
            if ((addr = alloc_from_new_buff(header, 0, tmp + 1)) != NULL) {                        //old buffer is full, create new buff and alloc
                return addr;
            } else {                                                                            //get new page,create new buff and alloc
                u64 sl = get_suggest_buff_len_by_header(header, tmp + 1);
                tmp = MEM_ROUND_UP(sl, PAGE_MASK) >> PAGE_SHIFT;
                if (malloc_sys_request_pages(tmp) == 0)
                    return NULL;
                if ((addr = alloc_from_new_buff(header, 0, tmp + 1)) != NULL) {
                    return addr;
                } else {
                    return NULL;
                }
            }
        }
    } else {
        if ((addr = alloc_from_new_buff(NULL, size, 0)) != NULL) {                                //no old buff
            return addr;
        } else {                                                                            //get new page,create new buff and alloc
            u64 sl = get_suggest_buff_len_by_header(NULL, 0);
            tmp = MEM_ROUND_UP(sl, PAGE_MASK) >> PAGE_SHIFT;
            if (malloc_sys_request_pages(tmp) == 0)
                return NULL;
            if ((addr = alloc_from_new_buff(NULL, size, 0)) != NULL) {
                return addr;
            } else {
                return NULL;
            }
        }
    }
}

void free(void* addr)
{
    u64 node_addr_s,node_addr_e,free_addr;
    free_addr = (u64)addr;
    Node* node = rbtree_biggest_smaller(&g_base->addr_root, addr, addr_rbtree_node_search_cmp);
    if (!node) {                    //this situation is a error!
        return;
    }
    struct malloc_block_header* node_header = container_of(node, struct malloc_block_header, addr_tree_node);
    node_addr_s = mbh_get_data_p(node_header);
    node_addr_e = mbh_get_gap_p(node_header);
    if (node_addr_e <= free_addr) {  //pointer to gap area, error
        return;
    }
    if ( (free_addr - node_addr_s) % mbh_get_itemlen(node_header)) {  //pointer not aligned by itemlen, error situation,wrong pointer
        return;
    }
    u16 idx = (free_addr - node_addr_s) / mbh_get_itemlen(node_header);
    dealloc_from_this_buff(node_header, idx);
}

u32 compress_va(u32 page)
{

}
