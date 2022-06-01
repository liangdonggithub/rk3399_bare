#include "asm/secure.h"
.section ".text"
.align 3
.global _secure_init
_secure_init:
    //set rk3399 ddr in bypass mode to alowed non-secure ddr access
	ldr x0,=(SGRF_BASE+SGRF_DDRRGN_CON0_16(16))
	ldr w1,[x0]
	ldr w2,=SGRF_DDR_RGN_BYPS
	orr w1,w1,w2
	str w1,[x0]
    //set all slave non-secure
    ldr x1,=(SGRF_BASE+SGRF_SLV_SECURE_CON0_4(SGRF_SLV_NUM))
    ldr x0,=(SGRF_BASE+SGRF_SLV_SECURE_CON0_4(0))
    ldr w2,=0xffff0000
0:
    str w2,[x0],#4
    cmp x0,x1
    b.ne 0b

    ldr x1,=(SGRF_BASE+SGRF_PMU_SLV_CON0_1(SGRF_PMU_SLV_NUM))
    ldr x0,=(SGRF_BASE+SGRF_PMU_SLV_CON0_1(0))
    ldr w2,=0xffff0000
1:
    str w2,[x0],#4
    cmp x0,x1
    b.ne 1b
    
    dmb SY
    ret