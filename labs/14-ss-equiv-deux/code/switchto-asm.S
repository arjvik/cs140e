#include "rpi-asm.h"

@ all of these are called from <switchto.h>: look there for more
@ help in understanding if need to.

@ drop in your code from last lab to get sp and lr.
@
@ void priv_get_sp_lr_asm(uint32_t mode, uint32_t *sp, uint32_t *lr);
@   - assumes caller is at privileged mode [we could check this]
@   - mode in r0
@   - sp pointer in r1
@   - lr pointer in r2
@
MK_FN(priv_get_sp_lr_asm)
    @ b staff_priv_get_sp_lr_asm

    mrs r4, cpsr
    msr cpsr_c, r0

    stm r1, {sp}
    stm r2, {lr}

    msr cpsr_c, r4
    
    bx lr

    asm_not_reached();


@ drop in your code from lab 13 [in 1-rfe-asm.S]
@
@ void switchto_user_asm(regs_t *r)  __attribute__((noreturn));
@  - <r> is a 17-entry register entry in r0.
@
@ switch to user mode loading r0-r15, and cpsr from <r>
MK_FN(switchto_user_asm)
    @ b staff_switchto_user_asm
    add sp, r0, #60
    ldm r0, {r0-r14}^
    rfe sp
    asm_not_reached();
    asm_not_reached();

@ drop in your code from lab 13
@ 
@ void switchto_priv_asm(regs_t *r)  __attribute__((noreturn));
MK_FN(switchto_priv_asm)
    @ b staff_switchto_priv_asm
    ldr r1, [r0, #64]
    msr cpsr_c, r1
    prefetch_flush(r1);
    ldm r0, {r0-r15} @ this pops the PC too lol
    asm_not_reached();

@ you'll have to write this, but it should mirror your other save
@ routines.    
@
@ void cswitchto_priv_asm(regs_t *old, regs_t *next);
@ 
@ voluntary context switch: save the current registers in <old>
@ and call the right switchto on <next>
MK_FN(cswitchto_priv_asm)
    @ initially just forward to ours.
    @ b staff_cswitchto_priv_asm

    @ to implement:
    @ 1. save all the callee regs (just like in your threads library)
    @    into the 17-entry register array we have been using.  note
    @    these have to be at the right offsets.
    @    e.g.:
    @       str r4,  [r0, #(4*4)]  @ r4
    @       str r5,  [r0, #(5*4)]  @ r5
    @       ... etc ...
    @   don't forget the cpsr!
    @ add r0, r0, #16
    stm r0, {r0-r14}
    str lr, [r0, #60]
    mrs	r3, cpsr
    str r3, [r0, #64]


    @ 2. just call the right switch routine: note, we should
    @    *not* reach this location when switching back!
    mov r0, r1
    bl switchto_priv_asm
    asm_not_reached();

@ you'll have to write this, but it should mirror your other save
@ routines.    
@
@ void cswitchto_user_asm(regs_t *old, regs_t *next);
@ 
@ voluntary context switch: save the current registers in <old>
@ and call the right switchto on <next>
MK_FN(cswitchto_user_asm)
    @ initially just forward to ours.
    @ b staff_cswitchto_user_asm

    @ asm_todo("save the registers: identical to <switchto_priv_asm>")
    
    stm r0, {r0-r14}
    str lr, [r0, #60]
    mrs	r3, cpsr
    str r3, [r0, #64]
    mov r0, r1
    bl switchto_user_asm
    asm_not_reached();
