#include "rpi-asm.h"

MK_FN(rfe_asm)
    rfe r0
    asm_not_reached();

MK_FN(rfe_trampoline_asm)
    mov r0, #INT_STACK_ADDR
    mov sp, #INT_STACK_ADDR
    bl rfe_c_code

MK_FN(blk_rfe_asm)
    add r0, r0, #15*4
    rfe r0
    asm_not_reached();

MK_FN(blk_trampoline_asm)
    bl rfe_trampoline_asm
    asm_not_reached();


MK_FN(switchto_user_asm)
    add sp, r0, #60
    ldm r0, {r0-r14}^
    rfe sp
    asm_not_reached();

MK_FN(switchto_priv_asm)
    ldr r1, [r0, #64]
    msr cpsr_cxsf, r1
    prefetch_flush(r1);
    ldm r0, {r0-r15} @ this pops the PC too lol
    asm_not_reached();

@ so we don't get linker errors 
.weak rfe_c_code
rfe_c_code:
    asm_not_reached()
