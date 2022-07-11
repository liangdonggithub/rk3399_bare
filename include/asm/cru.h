/*
 * @Author: Neal 624872416@qq.com
 * @Date: 2022-07-11 11:29:03
 * @LastEditors: Neal 624872416@qq.com
 * @LastEditTime: 2022-07-11 14:05:37
 * @FilePath: /rk3399_bare/include/asm/cru.h
 * @Description: cru related
 */
#ifndef CRU_H
#define CRU_H

#define CRU_BASE                    (0xff760000)

#define CRU_LPLL_CON_BASE           (CRU_BASE + 0x0)
#define CRU_LPLL_CON(x)             (CRU_LPLL_CON_BASE + (x) * 4)           //x from 0-5

#define CRU_BPLL_CON_BASE           (CRU_BASE + 0x20)
#define CRU_BPLL_CON(x)             (CRU_BPLL_CON_BASE + (x) * 4)

#define CRU_DPLL_CON_BASE           (CRU_BASE + 0x40)
#define CRU_DPLL_CON(x)             (CRU_DPLL_CON_BASE + (x) * 4)

#define CRU_CPLL_CON_BASE           (CRU_BASE + 0x60)
#define CRU_CPLL_CON(x)             (CRU_CPLL_CON_BASE + (x) * 4)

#define CRU_GPLL_CON_BASE           (CRU_BASE + 0x80)
#define CRU_GPLL_CON(x)             (CRU_GPLL_CON_BASE + (x) * 4)

#define CRU_NPLL_CON_BASE           (CRU_BASE + 0xa0)
#define CRU_NPLL_CON(x)             (CRU_NPLL_CON_BASE + (x) * 4)

#define CRU_VPLL_CON_BASE           (CRU_BASE + 0xc0)
#define CRU_VPLL_CON(x)             (CRU_VPLL_CON_BASE + (x) * 4)

#define CRU_CLKSEL_CON_BASE         (CRU_BASE + 0x0100)
#define CRU_CLKSEL_CON(x)           (CRU_CLKSEL_CON_BASE + (x) * 4)         //x from 0-107

#define CRU_CLKGATE_CON_BASE        (CRU_BASE + 0x0300)
#define CRU_CLKGATE_CON(x)          (CRU_CLKGATE_CON_BASE + (x) * 4)        //x from 0-34

#define CRU_SOFTRST_CON_BASE        (CRU_BASE + 0x0400)
#define CRU_SOFTRST_CON(x)          (CRU_SOFTRST_CON_BASE + (x) * 4)        //x from 0-20

#define CRU_GLB_SRST_FST_VALUE      (CRU_BASE + 0x0500)
#define CRU_GLB_SRST_SND_VALUE      (CRU_BASE + 0x0504)
#define CRU_GLB_CNT_TH              (CRU_BASE + 0x0508)
#define CRU_MISC_CON                (CRU_BASE + 0x050c)
#define CRU_GLB_RST_CON             (CRU_BASE + 0x0510)
#define CRU_GLB_RST_ST              (CRU_BASE + 0x0514)

#define CRU_SDMMC_CON0              (CRU_BASE + 0x0580)
#define CRU_SDMMC_CON1              (CRU_BASE + 0x0584)
#define CRU_SDIO0_CON0              (CRU_BASE + 0x0588)
#define CRU_SDIO0_CON1              (CRU_BASE + 0x051c)

#endif
