#ifndef HELPER_H
#define HELPER_H

#define offsetof(TYPE, MEMBER) ((unsigned long)&((TYPE *)0)->MEMBER)

#define container_of(ptr, type, member) ({                      \
     const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
     (type *)( (char *)__mptr - offsetof(type,member) );})

#define MEM_ROUND_DOWN(addr,mask)   (addr & (~mask))
#define MEM_ROUND_UP(addr,mask)     ((addr&mask)?((addr&(~mask))+mask+1):((!addr)?(mask+1):(addr)))

#endif