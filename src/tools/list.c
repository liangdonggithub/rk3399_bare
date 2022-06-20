#include "tools/list.h"

struct list_head* list_add_tail(struct list_head* p_head, struct list_head* p_item)
{
    struct list_head* p_lastitem;
    if (!p_head || !p_item || p_head == p_item)
        return NULL;
    if (!p_head->p_prev && !p_head->p_next) {
        p_head->p_prev = p_item;
        p_head->p_next = p_item;
        p_item->p_prev = p_head;
        p_item->p_next = p_head;
    } else if (p_head->p_prev && p_head->p_next) {
        p_lastitem = p_head->p_prev;
        p_head->p_prev = p_item;
        p_lastitem->p_next = p_item;
        p_item->p_prev = p_lastitem;
        p_item->p_next = p_head;
    }
    else {
        return NULL;
    }
    return p_item;
}

struct list_head* list_add_head(struct list_head* p_head, struct list_head* p_item)
{
    struct list_head* p_firstitem;
    if (!p_head || !p_item || p_head == p_item)
        return NULL;
    if (!p_head->p_prev && !p_head->p_next) {
        p_head->p_prev = p_item;
        p_head->p_next = p_item;
        p_item->p_prev = p_head;
        p_item->p_next = p_head;
    } else if (p_head->p_prev && p_head->p_next) {
        p_firstitem = p_head->p_next;
        p_head->p_next = p_item;
        p_item->p_next = p_firstitem;
        p_item->p_prev = p_head;
        p_firstitem->p_prev = p_item;
    }
    else {
        return NULL;
    }
    return p_item;
    
}

struct list_head* list_insert(struct list_head* p_head, struct list_head* p_prev, struct list_head* p_item)
{
    struct list_head* p_tmp;
    if (!p_head || !p_prev || !p_item)
        return NULL;
    if (p_head == p_prev || p_head == p_item || p_prev == p_item)
        return NULL;
    if (!p_head->p_prev && !p_head->p_next)            //p_prev must not in p_head list, unnormal!
        return NULL;
    else if (!p_prev->p_prev && !p_prev->p_next)        //p_prev must not in p_head list, unnormal!
        return NULL;
    else if (p_head->p_prev && p_head->p_next) {        //don't check p_prev in p_head list or not. That's too slow!
        p_tmp = p_prev->p_next;
        p_prev->p_next = p_item;
        p_item->p_prev = p_prev;
        p_item->p_next = p_tmp;
        p_tmp->p_prev = p_item;
    } else {
        return NULL;
    }
    return p_item;
}

struct list_head* list_remove(struct list_head* p_head, struct list_head* p_item)
{
    struct list_head* p_prev,*p_next;
    if (!p_head || !p_item || p_head == p_item)
        return NULL;
    if (!p_head->p_prev && !p_head->p_next)            //p_item must not in p_head list, unnormal!
        return NULL;
    else if (!p_item->p_prev && !p_item->p_next)        //p_item must not in p_head list, unnormal!
        return NULL;
    else if (p_head->p_prev && p_head->p_next && p_item->p_prev && p_item->p_next) {        //don't check p_item in p_head list or not. That's too slow!
        if (p_head->p_next == p_item && p_head->p_prev == p_item) {                         //p_head list has only one item
            p_head->p_next = p_head->p_prev = NULL;
            p_item->p_next = p_item->p_prev = NULL;     //for usage safe
        } else {
            p_prev = p_item->p_prev;
            p_next = p_item->p_next;
            p_prev->p_next = p_next;
            p_item->p_prev = NULL;                      //for usage safe
            p_item->p_next = NULL;                      //for usage safe
            p_next->p_prev = p_prev;
        }
    } else {
        return NULL;
    }
    return p_item;
}

u8 list_empty(struct list_head* p_head)
{
    return (!p_head->p_next && !p_head->p_prev) ? 1 : 0;
}
