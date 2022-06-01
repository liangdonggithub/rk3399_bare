#include "sysreg.h"
.section ".text"
.align 2

.global _start
_start:
	mrs x0,mpidr_el1
	and x0,x0,#0xff
	cbnz x0,slave_cpu
	ldr	w0,=GRF_GPIO4B_IOMUX
	ldr w1,=GRF_JTAG_VAL
	str w1,[x0]
	ldr w0,=GRF_SOC_CON7
	ldr w1,=FORCE_JTAG
	str w1,[x0]
1:
	b 1b

slave_cpu:
	wfe
	ldr x1,=CPU_RELEASE_ADDR
	ldr x0,[x1]
	cbz x0,slave_cpu
	br	x0
