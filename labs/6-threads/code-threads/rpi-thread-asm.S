/*
 * implement these.  ordered in difficulty.
 */
#include "rpi-asm.h"

@ return the current stack pointer.
MK_FN(rpi_get_sp)
    mov r0, sp
    bx lr

MK_FN(rpi_cswitch)
    push {r4-r11, lr}
    str sp, [r0]
    mov sp, r1
    pop {r4-r11, lr}
    bx lr

@ [Make sure you can answer: why do we need to do this?]
@
@ use this to setup each thread for the first time.
@ setup the stack so that when cswitch runs it will:
@	- load address of <rpi_init_trampoline> into LR
@	- <code> into r1, 
@	- <arg> into r0
@ 
MK_FN(rpi_init_trampoline)
    @ 2. Implement `rpi_init_trampoline` in `thread-asm.S` so that
    @  it loads `arg` from the stack (from Step 1) into `r0`, 
    @  loads `code` into another register that it then uses to 
    @  do a branch and link.
    mov r0, r5
    mov r1, r4
    blx r1
    bl rpi_exit
    bx lr
