#ifndef REG_COMMON_H
#define REG_COMMON_H

#define AREA_SET(val,shift,bits)                    ((val & ((1UL<<bits)-1)) << shift)
#define AREA_SE_SET(val,start,end)                  ((val & ((1UL<<(end - start + 1))-1)) << start)
#define BIT_SET(val,shift)                          ((val & 1UL) << shift)
#define AREA_GET(val,shift,bits)                    ((val >> shift) & ((1UL<<bits)-1))
#define AREA_SE_GET(val,start,end)                  ((val >> start) & ((1UL<<(end - start + 1)) - 1))
#define BIT_GET(val,shift)                          ((val >> shift) & 1)

#define read_sysreg(reg) ({ \
    unsigned long _val; \
    asm volatile("mrs %0," #reg \
    : "=r"(_val)); \
    _val; \
})

#define write_sysreg(val, reg) ({ \
    unsigned long _val = (unsigned long)val; \
    asm volatile("msr " #reg ", %x0" \
    :: "rZ"(_val)); \
})

#endif