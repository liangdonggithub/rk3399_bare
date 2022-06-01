#include "sysinfo.h"
#include "asm/secure.h"
.section ".text"
.align 3

.global _start
_start:
#ifdef SYS_INFO_ALLOW_SWJ
	ldr x0,=0xff77e024
	ldr w1,=0xf000a0
	str w1,[x0]
	ldr x0,=0xff77e21c
	ldr w1,=0x10001000
	str w1,[x0]
#endif
	//set gic controller
	bl _gic_init
	//secure init
	bl _secure_init
	//switch exception level to el1 for kernel init
	mrs x0,CurrentEL
	lsr x0,x0,#2
	and x0,x0,#0x3
	cmp x0,#1
	b.eq el1_entry
	cmp x0,#2
	b.eq el2_to_el1

el3_to_el2:
	mrs x0,CurrentEL

	msr cptr_el3, xzr           //Enable FP/SIMD

	ldr x0,=((3<<16)|(3<<20))
	msr cpacr_el1,x0

	ldr x0, =24000000           //24MHz
	msr cntfrq_el0, x0          //Initialize CNTFRQ

	msr sctlr_el2,xzr
	msr hcr_el2,xzr

	mrs x0,scr_el3
	orr x0,x0,#(1<<10)			//RW lower level all aarch64
	orr x0,x0,#(1<<0)			//NS lower level all non-secure
	msr scr_el3,x0
	mov x0,#0b01001				//el2h
	msr spsr_el3,x0

	adr x0,el2_to_el1
	msr elr_el3,x0

	eret

el2_to_el1:
	mrs x0,CurrentEL
	msr sctlr_el1,xzr
	mrs x0,hcr_el2
	orr x0,x0,#(1<<31)			//RW el1 is aarch64
	msr hcr_el2,x0
	mov x0,#0b00101				//el1h
	msr spsr_el2,x0

	adr x0,el1_entry
	msr elr_el2,x0

	eret

el1_entry:
	ldr x0,=SYS_INFO_MEM_SIZE	//set stack pointer
	bic sp,x0,#0xf

	ldr x0,=vectors				//set vectors
	msr vbar_el1,x0

	ldr x0,=__bss_start
	ldr x1,=__bss_end
	cmp x0,x1
	b.eq 1f
0:
	str xzr,[x0],#8
	cmp x0,x1
	b.ne 0b
1:
	b main

	
