.section ".text"
.align 2
.global jump_to_kern
jump_to_kern:
    mov x29,x30
    mov x6,x0
    mov x7,x1
    bl fix_relc_dyn
    bl fix_got
    mov sp,x7
    adr x1,kern
    add x1,x1,x0
    isb
    br x1
kern:
    add x29,x29,x0
    mov x30,x29
    ret

fix_relc_dyn:
    adr x1,__init_reladyn_s
    adr x2,__init_reladyn_e
    cmp x1,x2
    b.eq 1f
0:
    ldp x3,x4,[x1],#16
    ldr x5,[x1],#8
    cmp x4,#0x403
    b.ne 0b
    add x5,x5,x0
    str x5,[x3]
    cmp x1,x2
    b.ne 0b
1:
    ret

fix_got:
    adr x1,__init_got_s
    adr x2,__init_got_e
    cmp x1,x2
    b.eq 1f
    ldr x5,[x1]
    add x5,x5,x0
    str x5,[x1]
1:
    ret

