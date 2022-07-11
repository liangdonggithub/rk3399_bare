/*
 * @Author: Neal 624872416@qq.com
 * @Date: 2022-06-13 10:52:21
 * @LastEditors: Neal 624872416@qq.com
 * @LastEditTime: 2022-07-05 11:41:14
 * @FilePath: /rk3399_bare/include/tools/helper.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef HELPER_H
#define HELPER_H

#define offsetof(TYPE, MEMBER) ((unsigned long)&((TYPE *)0)->MEMBER)

#define container_of(ptr, type, member) ({                      \
     const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
     (type *)( (char *)__mptr - offsetof(type,member) );})

#define MEM_ROUND_DOWN(addr,mask)   ((addr) & (~(mask)))
#define MEM_ROUND_UP(addr,mask)     (((addr)&(mask))?(((addr)&(~(mask)))+(mask)+1):((!(addr))?((mask)+1):((addr))))

#endif