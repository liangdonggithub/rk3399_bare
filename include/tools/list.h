#ifndef LIST_H
#define LIST_H

#include "tools/helper.h"
#include "tools/types.h"

struct list_head {
	struct list_head*	p_prev;
	struct list_head*	p_next;
};

#define LIST_HEAD_INIT(name)			struct list_head #name; #name.p_prev = NULL; #name.p_next = NULL;

#define list_entry(ptr, type, member) container_of(ptr,type,member) 

#define list_for_each_entry(pos, head, member)		\
	for (pos = list_entry((head)->p_next,typeof(*pos),member);	\
		&pos->member!=(head);	\
		pos=list_entry(pos->member.p_next,typeof(*pos),member))

#define list_for_each_entry_safe(pos,n,head,member)	\
	for (pos = list_entry((head)->p_next, typeof(*pos), member), n = list_entry(pos->member.p_next, typeof(*pos), member);	\
		&pos->member != (head);	\
		pos = n, n = list_entry(n->member.p_next, typeof(*n), member))

#define LIST_FOR_EACH_SAFE(head,item)	

u8 list_empty(struct list_head* p_head);
struct list_head* list_add_head(struct list_head* p_head, struct list_head* p_item);
struct list_head* list_add_tail(struct list_head* p_head, struct list_head* p_item);
struct list_head* list_insert(struct list_head* p_head, struct list_head* p_prev, struct list_head* p_item);
struct list_head* list_remove(struct list_head* p_head, struct list_head* p_item);

#endif