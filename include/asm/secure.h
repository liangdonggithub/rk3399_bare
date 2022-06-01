#ifndef SECURE_H
#define SECURE_H

#define SGRF_BASE                   0xff330000
#define SGRF_DDRRGN_CON0_16(n)      ((n) * 4)
#define SGRF_DDR_RGN_BYPS           ((1<<9)|(1<<(9+16)))
#define SGRF_SLV_SECURE_CON0_4(n)   (0xe3c0 + ((n) - 0) * 4)
#define SGRF_PMU_SLV_CON0_1(n)      (0xc240 + ((n) - 0) * 4)
#define SGRF_SLV_NUM                5
#define SGRF_PMU_SLV_NUM            2

#endif 