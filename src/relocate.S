.section ".text"
.align 2
.global init_relc_dyn
init_relc_dyn:
    mov x29,x30
    adr x0,__init_reladyn_s
    adr x1,__init_reladyn_e
    cmp x0,x1
    b.eq out
0:
    ldp x2,x3,[x0],#16
    ldr x4,[x0],#8
    str x4,[x2]
    cmp x0,x1
    b.ne 0b
out:
    mov x30,x29
    ret
