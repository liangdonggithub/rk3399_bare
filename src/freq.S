#include "asm/cru.h"

.section ".text"
.align 2
.global early_freq_set
early_freq_set:
    ldr x10,=CRU_LPLL_CON(0)
    ldr w0,[x10]
    ldr x10,=CRU_LPLL_CON(1)
    ldr w1,[x10]
    ldr x10,=CRU_LPLL_CON(2)
    ldr w2,[x10]
    ldr x10,=CRU_LPLL_CON(3)
    ldr w3,[x10]
    ldr x10,=CRU_LPLL_CON(4)
    ldr w4,[x10]
    ldr x10,=CRU_LPLL_CON(5)
    ldr w5,[x10]
    ldr x10,=CRU_CLKSEL_CON(0)
    ldr w0,[x10]
    ldr x10,=CRU_CLKSEL_CON(1)
    ldr w1,[x10]
    ldr x10,=CRU_CLKGATE_CON(0)
    ldr w0,[x10]

    ldr x10,=CRU_LPLL_CON(1)
    ldr w0,[x10]
    bic w0,w0,#0x700
    mov w1,#0xffff0200
    orr w0,w0,w1
    str w0,[x10]
    dmb SY
    ldr w0,[x10]
    nop
    nop
    nop
    ldr x10,=CRU_LPLL_CON(2)
0:
    ldr w0,[x10]
    tst w0,#31
    beq 0b

    ret