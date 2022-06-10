#ifndef VMSA_H
#define VMSA_H

#include "reg_common.h"
#include "sysinfo.h"

//l3 page table item attrs
#define PTI_AttrIndx_SET(val)                     	AREA_SET(val,2,3)
#define PTI_NS_SET(val)                           	BIT_SET(val,5)
#define PTI_AP_SET(val)                           	AREA_SET(val,6,2)
#define PTI_SH_SET(val)                           	AREA_SET(val,8,2)
#define PTI_AF_SET(val)                           	BIT_SET(val,10)
#define PTI_nG_SET(val)								BIT_SET(val,11)
#define PTI_nT_SET(val)								BIT_SET(val,16)
#define PTI_GP_SET(val)								BIT_SET(val,50)
#define PTI_DBM_SET(val)							BIT_SET(val,51)
#define PTI_Contiguous_SET(val)						BIT_SET(val,52)
#define PTI_PXN_SET(val)							BIT_SET(val,53)
#define PTI_UXN_SET(val)							BIT_SET(val,54)
#define PTI_PBHA_SET(val)							AREA_SET(val,59,4)
#define PTI_PFN_SET(val)							AREA_SE_SET(val,12,47)
#define PTI_PTYPE_SET(val)							AREA_SET(val,0,2)

#define PTI_PTYPE_IS_VAL							3
#define PTI_PTYPE_IS_RES							1
#define PTI_PTYPE_IS_INV							0

#define PTI_ATTRIDX_DEV_nGnRnE    					0
#define PTI_ATTRIDX_DEV_nGnRE     					1
#define PTI_ATTRIDX_DEV_GRE       					2
#define PTI_ATTRIDX_NOR_NC        					3
#define PTI_ATTRIDX_NOR								4
#define PTI_ATTRIDX_NOR_WT        					5

//l0-l2 page table item attrs
#define PTI_NEXTL_TYPE_SET(val)						AREA_SET(val,0,2)
#define PTI_NEXTL_ADDR_SET(val)						AREA_SE_SET(val,12,47)
#define PTI_NEXTL_IS_PTI							3
#define PTI_NEXTL_IS_BLK							1
#define PTI_NEXTL_IS_INV							0

//tcr_el1
#define TCR_T0SZ_SET(val)							AREA_SE_SET(val,0,5)
#define TCR_TG0_SET(val)							AREA_SE_SET(val,14,15)
#define TCR_T1SZ_SET(val)							AREA_SE_SET(val,16,21)
#define TCR_TG1_SET(val)							AREA_SE_SET(val,30,31)
#define TCR_IPS_SET(val)							AREA_SE_SET(val,32,34)

#define TCR_VABITS_TO_SZ(val)						(64-val)
#define TCR_TG0_4K									0
#define TCR_TG0_64K									1
#define TCR_TG0_16K									2
#define TCR_TG1_16K									1
#define TCR_TG1_4K									2
#define TCR_TG1_64K									3

//ID_AA64MMFR0_EL1
#define ID_AA64MMFR0_GET_PARange(val)				AREA_SE_GET(val,0,3)
#define ID_AA64MMFR0_GET_TGran16(val)				AREA_SE_GET(val,20,23)
#define ID_AA64MMFR0_GET_TGran64(val)				AREA_SE_GET(val,24,27)
#define ID_AA64MMFR0_GET_TGran4(val)				AREA_SE_GET(val,28,31)

#define ID_AA64MMFR0_PARANGE_48						0x5
#define ID_AA64MMFR0_PARANGE_52						0x6
#define ID_AA64MMFR0_TGran4_Spt						0x0

//sctlr_el1
#define SCTLR_ELx_M (1<<0)

#endif