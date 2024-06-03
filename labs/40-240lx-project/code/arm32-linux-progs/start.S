.section .text
.global _start

_start:
    // Set up stack pointer if needed (usually done by the loader)
    // mov sp, #0x80000    // Example: set SP to 0x80000

    // Call main function
    bl truemain

    // Call exit system call
    mov r7, #1          // syscall number for exit (sys_exit)
    mov r0, #0          // exit code 0
    swi 0               // make the system call
