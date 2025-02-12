#include "rpi-asm.h"

@ save all the general registers and the spsr in 
@ ascending order onto a block of memory on the
@ stack.  the pointer to the base of this to 
@ do_syscall (defined in `2-reg-save.c` and 
@ `2-reg-save-full.c`).
@
swi_trampoline:
    sub sp, sp, #68
    stm sp, {r0-r14}^
    
    str lr, [sp, #60]
    
    mrs r4, spsr
    str r4, [sp, #64]

    mov r0, sp

    b do_syscall

    @ note: this trampoline does not resume.
    asm_not_reached();

@ only handle single step.
.align 5;
.globl swi_test_handlers
swi_test_handlers:
    b unhandled_reset
    b unhandled_undefined_instruction
    b swi_trampoline
    b unhandled_prefetch_abort 
    b unhandled_data_abort
    b unhandled_interrupt
    b unhandled_fiq


.weak do_syscall
do_syscall:
    asm_not_reached();
