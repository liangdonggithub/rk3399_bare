/*
 * @Author: Neal 624872416@qq.com
 * @Date: 2022-06-13 16:57:13
 * @LastEditors: Neal 624872416@qq.com
 * @LastEditTime: 2022-07-06 14:47:15
 * @FilePath: /rk3399_bare/include/mm/malloc.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef MALLOC_H
#define MALLOC_H

#include "tools/types.h"
#include "tools/rbtree.h"
#include "tools/list.h"
#include "sysinfo.h"
#include "mm/page_allocator.h"

struct malloc_block_header {

#define FIRST_SUGGEST_ITEMNUM(itemlen)  ((4096/itemlen) ? (4096/itemlen) : 1)
#define MAX_SUGGEST_ITEMNUM(itemlen)    ((0x1000000/itemlen) ? (0x1000000/itemlen) : 1)
#define MAX_VA                          (KERN_STACK_REGION_S - KERN_INIT_STACK_PAGES * PAGE_SIZE)

#define ATTR_VA_FIRST                   (1<<0)
#define ATTR_VA_LAST                    (1<<1)
#define ATTR_BUFF_FIRST                 (1<<2)
#define ATTR_FULL                       (1<<3)
#define ATTR_NOT_CLEAR_IMED             (1<<4)
#define ATTR_NOT_USED                   (1<<5)

#define mbh_get_status(mbh)             ((mbh->attr) & 0xffff)
#define mbh_get_itemnum(mbh)            (((mbh->attr)>>16) & 0xffff)
#define mbh_get_itemlen(mbh)            (((mbh->attr)>>32) & 0xffffffff)
#define mbh_get_idx_map(mbh,idx)        ((mbh->maps[idx/64]>>(idx%64))&0x1)
#define mbh_get_extra_len(mbh)          (sizeof(struct malloc_block_header) + (mbh_get_itemnum(mbh)/64 + 1) * 64)
#define mbh_get_extra_len_by_num(num)   (sizeof(struct malloc_block_header) + (num/64 + 1) * 64)
#define mbh_get_data_buff_len(mbh)      (mbh_get_itemnum(mbh) * mbh_get_itemlen(mbh))
#define mbh_get_data_p(mbh)             ((u64)mbh + mbh_get_extra_len(mbh))
#define mbh_get_gap_p(mbh)              ((u64)mbh + mbh_get_extra_len(mbh) + mbh_get_data_buff_len(mbh))

#define mbh_set_status(mbh,val)         do { mbh->attr &= ~0xffffUL; mbh->attr |= val&0xffffUL; }while(0)
#define mbh_set_itemnum(mbh,val)        do { mbh->attr &= ~(0xffffUL<<16); mbh->attr |= (val&0xffffUL)<<16; }while(0)
#define mbh_set_itemlen(mbh,val)        do { mbh->attr &= ~(0xffffffffUL<<32); mbh->attr |= (val&0xffffffffUL)<<32; }while(0)
#define mbh_set_idx_map(mbh,idx,val)    do { mbh->maps[idx/64] &= ~(1UL<<(idx%64)); mbh->maps[idx/64] |= (u64)val<<(idx%64); }while(0)
#define mbh_clr_map(mbh,tmp)            do { \
                                            tmp = 0; \
                                            while (tmp < mbh_get_itemnum(mbh)/64 + 1) { \
                                                mbh->maps[tmp] = 0; \
                                                tmp++; \
                                            } \
                                        } while (0)

    struct RBTreeNode       addr_tree_node;
    struct RBTreeNode       gap_tree_node;
    struct RBTreeNode       buff_tree_node;
    struct RBTreeNode       nonmgtva_tree_node;
    struct list_head        pos_list;
    struct list_head        buff_list;
    struct malloc_block_header* buff_head;
    u64                     non_mgt_va_page;
    u64                     mgt_max_va;
    u64                     attr;
    u64                     maps[0];
};// this struct must aligned by 8 byte

struct malloc_block_base {
    struct rb_root          gap_root;
    struct rb_root          addr_root;
    struct rb_root          buff_root;
    struct rb_root          nonmgtva_root;
    struct list_head        pos_head;
};

struct malloc_block_base* init_malloc_sys(void);

void* malloc(u64 size);

void free(void* addr);

u32 compress_va(u32 page);

#endif