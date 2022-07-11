/*
 * @Author: Neal 624872416@qq.com
 * @Date: 2022-07-05 10:30:12
 * @LastEditors: Neal 624872416@qq.com
 * @LastEditTime: 2022-07-05 10:30:58
 * @FilePath: /rk3399_bare/include/sysinfo_c.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#ifndef SYSINFO_C_H
#define SYSINFO_C_H

enum SUB_SYSTEM{
    SUBS_KERN = 0,
    SUBS_PAGE_TABLE,
    SUBS_PAGE_ALLOC,
    SUBS_HEAP,
    SUBS_STACK,
    SUBS_MAX,
};

#endif