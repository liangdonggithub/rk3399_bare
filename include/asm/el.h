#ifndef EL_H
#define EL_H
//SCR_EL3
#define SCR_EL3_RES1_ALL1 (3<<4)
#define SCR_EL3_BIT_ST  11
#define SCR_EL3_BIT_RW  10
#define SCR_EL3_BIT_HCE 8
#define SCR_EL3_BIT_SMD 7
#define SCR_EL3_BIT_EA  3
#define SCR_EL3_BIT_NS  0
#define SCR_EL3_INIT    ((1<<SCR_EL3_BIT_HCE)|(1<<SCR_EL3_BIT_RW)|(1<<SCR_EL3_BIT_NS)|(0<<SCR_EL3_BIT_SMD)|SCR_EL3_RES1_ALL1)
//HCR_EL2
#define HCR_EL2_BIT_RW  31
#define HCR_EL2_INIT    (1<<HCR_EL2_BIT_RW)
//SCTLR_EL2
#define SCTLR_EL2_RES1_ALL1 ((1<<29)|(1<<28)|(1<<23)|(1<<22)|(1<<18)|(1<<16)|(1<<11)|(1<<5)|(1<<4))
#define SCTLR_EL2_RES0_ALL0 ((0<<31)|(0<<30)|(0<<27)|(0<<26)|(0<<24)|(0<<21)|(0<<20)|(0<<17)|(0<<15)|(0<<14)|(0<<13)|(0<<10)|(0<<9)|(0<<8)|(0<<7)|(0<<6))
#define SCTLR_EL2_BIT_EE	25
#define SCTLR_EL2_BIT_WXN   19
#define SCTLR_EL2_BIT_I     12
#define SCTLR_EL2_BIT_SA    3
#define SCTLR_EL2_BIT_C     2
#define SCTLR_EL2_BIT_A     1
#define SCTLR_EL2_BIT_M		0
#define SCTLR_EL2_INIT		SCTLR_EL2_RES1_ALL1
//SCTLR_EL1
#define SCTLR_EL1_BIT_EE    25
#define SCTLR_EL1_BIT_E0E   24
#define SCTLR_EL1_BIT_I     12
#define SCTLR_EL1_BIT_C     2
#define SCTLR_EL1_BIT_M     0
#define SCTLR_EL1_INIT      ((0<<SCTLR_EL1_BIT_EE)|(0<<SCTLR_EL1_BIT_E0E)|(0<<SCTLR_EL1_BIT_M)) //el1 le el0 le
//spsr_elx
#define SPSR_ELx_AREA_M_SHIFT       0
#define SPSR_ELx_AREA_M_MASK        0xf
#define SPSR_ELx_AREA_DAIF_SHIFT    6
#define SPSR_ELx_AREA_DAIF_MASK     0xf
#define SPSR_ELx_AREA_NZCV_SHIFT    28
#define SPSR_ELx_AREA_NZCV_MASK     0xf
#define SPSR_ELx_BIT_M              4

#define SPSR_EL3_INIT               ((SPSR_ELx_AREA_DAIF_MASK<<SPSR_ELx_AREA_DAIF_SHIFT)|(9<<SPSR_ELx_AREA_M_SHIFT)|(0<<SPSR_ELx_BIT_M))
#define SPSR_EL3_TO_EL1_INIT        ((SPSR_ELx_AREA_DAIF_MASK<<SPSR_ELx_AREA_DAIF_SHIFT)|(5<<SPSR_ELx_AREA_M_SHIFT)|(0<<SPSR_ELx_BIT_M))
#define SPSR_EL2_INIT				((SPSR_ELx_AREA_DAIF_MASK<<SPSR_ELx_AREA_DAIF_SHIFT)|(5<<SPSR_ELx_AREA_M_SHIFT)|(0<<SPSR_ELx_BIT_M))
#define SPSR_EL1_INIT               ((SPSR_ELx_AREA_DAIF_MASK<<SPSR_ELx_AREA_DAIF_SHIFT)|(5<<SPSR_ELx_AREA_M_SHIFT)|(0<<SPSR_ELx_BIT_M))

#endif
