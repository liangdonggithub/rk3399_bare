#include "asm/gic.h"
#include "asm/macro.h"

.section ".text"
.align 2

.global _gic_init
_gic_init:
    /*******************************************************/
	mov	x29, x30		             //Save LR 

	ldr	x0, =RKIO_GICD_PHYS          //RKIO_GICD_PHYS   GIC DIST 
	bl	gic_init_secure

	ldr	x0, =RKIO_GICR_PHYS          //RKIO_GICR_PHYS
	bl	gic_init_secure_percpu

	mov	x30, x29			         //Restore LR
	ret	
	
/*******************************************************/	
//ref: u-boot/arch/arm/lib/gic_64.S

	/*Initialize Distributor  x0: Distributor Base*/
gic_init_secure:
	mov	w9, #0x22		         //EnableGrp1NS & ARE_NS
	str	w9, [x0, 0x0000]	     //Secure GICD_CTLR 
	ldr	w9, [x0, 0x0004]
	and	w10, w9, #0x1f		     //ITLinesNumber 
	cbz	w10, 1f			         //No SPIs 
	add	x11, x0, (0x0080 + 4)
	add	x12, x0, (0x0d00 + 4)
	mov	w9, #~0
0:	str	w9, [x11], #0x4
	str	wzr, [x12], #0x4	     //Config SPIs as Group1NS 
	sub	w10, w10, #0x1
	cbnz	w10, 0b
1:
	ret
	
	
	/*Initialize ReDistributor  x0: ReDistributor Base*/
gic_init_secure_percpu:
	mrs	x10, mpidr_el1
	lsr	x9, x10, #32
	bfi	x10, x9, #24, #8	    //w10 is aff3:aff2:aff1:aff0 
	mov	x9, x0
1:	ldr	x11, [x9, 0x0008]
	lsr	x11, x11, #32		    //w11 is aff3:aff2:aff1:aff0 
	cmp	w10, w11
	b.eq	2f
	add	x9, x9, #(2 << 16)
	b	1b

	/* x9: ReDistributor Base Address of Current CPU */
2:	mov	w10, #~0x2
	ldr	w11, [x9, 0x0014]
	and	w11, w11, w10		    //Clear ProcessorSleep 
	str	w11, [x9, 0x0014]
	dsb	st
	isb
3:	ldr	w10, [x9, 0x0014]
	tbnz	w10, #2, 3b		    //Wait Children be Alive 

	add	x10, x9, #(1 << 16)   	//SGI_Base 
	mov	w11, #~0
	str	w11, [x10, 0x0080]
	str	wzr, [x10, 0x0d00]	    //SGIs|PPIs Group1NS 
	mov	w11, #0x1		        //Enable SGI 0 
	str	w11, [x10, 0x0100]

	/* Initialize Cpu Interface */
	/* rockchip: first check elx for running on different el */
	switch_el x0, el3_sre, el2_sre, el1_sre

el3_sre:
	mrs	x10, S3_6_C12_C12_5
	orr	x10, x10, #0xf		     //SRE & Disable IRQ/FIQ Bypass & 
					             //Allow EL2 access to ICC_SRE_EL2 
	msr	S3_6_C12_C12_5, x10
	isb

el2_sre:
	mrs	x10, S3_4_C12_C9_5
	orr	x10, x10, #0xf		     //SRE & Disable IRQ/FIQ Bypass & 
				             	 //Allow EL1 access to ICC_SRE_EL1 
	msr	S3_4_C12_C9_5, x10
	isb

el1_sre:
	mrs	x0, CurrentEL	         //check currentEL 
	cmp	x0, 0xC
	b.ne	el1_ctlr	         //currentEL != EL3 

el3_ctlr:
	mov	x10, #0x1		         //EnableGrp1NS
	msr	S3_6_C12_C12_7, x10
	isb

	msr	S3_6_C12_C12_4, xzr
	isb

el1_ctlr:
	mov	x10, #0x3		         //EnableGrp1NS | EnableGrp1S 
	msr	S3_0_C12_C12_7, x10
	isb

	msr	S3_0_C12_C12_4, xzr    	 //NonSecure ICC_CTLR_EL1 
	isb

	mov	x10, #0xf0		         //Non-Secure access to ICC_PMR_EL1 
	msr	S3_0_C4_C6_0, x10
	isb	

	ret