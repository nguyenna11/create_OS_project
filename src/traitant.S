    .section .text
    .align 2
    .globl mon_traitant
    .type mon_traitant, @function
    .extern trap_handler

mon_traitant:
    addi sp, sp, -128
    sd ra,   0(sp)
    sd a0,   8(sp)
    sd a1,  16(sp)
    sd a2,  24(sp)
    sd a3,  32(sp)
    sd a4,  40(sp)
    sd a5,  48(sp)
    sd a6,  56(sp)
    sd a7,  64(sp)
    sd t0,  72(sp)
    sd t1,  80(sp)
    sd t2,  88(sp)
    sd t3,  96(sp)
    sd t4, 104(sp)
    sd t5, 112(sp)
    sd t6, 120(sp)

    csrr a0, mcause
    csrr a1, mie
    csrr a2, mip
    call trap_handler

    ld ra,   0(sp)
    ld a0,   8(sp)
    ld a1,  16(sp)
    ld a2,  24(sp)
    ld a3,  32(sp)
    ld a4,  40(sp)
    ld a5,  48(sp)
    ld a6,  56(sp)
    ld a7,  64(sp)
    ld t0,  72(sp)
    ld t1,  80(sp)
    ld t2,  88(sp)
    ld t3,  96(sp)
    ld t4, 104(sp)
    ld t5, 112(sp)
    ld t6, 120(sp)
    addi sp, sp, 128
    mret

    .size mon_traitant, .-mon_traitant
