#include "rpi-asm.h"

@ save all the general registers and the spsr in 
@ ascending order onto a block of memory on the
@ stack.  the pointer to the base of this to 
@ do_syscall (defined in `2-reg-save.c` and 
@ `2-reg-save-full.c`).
@
priv_swi_trampoline:
    sub sp, sp, #68


    stm sp, {r0-r7}
    str lr, [sp, #60]
    
    mrs r4, spsr
    str r4, [sp, #64]

    add r0, sp, #32

    mrs r2, cpsr
    mrs r1, spsr
    msr cpsr_c, r1
    prefetch_flush(r1)
    
    stm r0, {r8-r14}
    
    msr cpsr_c, r2
    prefetch_flush(r2)

    mov r0, sp

    b do_syscall

    @ note: this trampoline does not resume.
    asm_not_reached();

@ only handle single step.
.align 5;
.globl priv_swi_test_handlers
priv_swi_test_handlers:
    b unhandled_reset
    b unhandled_undefined_instruction
    b priv_swi_trampoline
    b unhandled_prefetch_abort 
    b unhandled_data_abort
    b unhandled_interrupt
    b unhandled_fiq