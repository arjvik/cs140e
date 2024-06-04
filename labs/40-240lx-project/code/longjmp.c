#include "longjmp.h"
#include "rpi.h"

struct jmp_buf env;
bool jmp_set = false;


// Moving jump state to globals for ease of use - surely no code smells here!
// int setjmp(struct jmp_buf *env) {}
// void longjmp(struct jmp_buf *env, int val) {}

int setjmp() {
    jmp_set = true;
    asm volatile(
        "str r4, [%0]\n"       // Store r4 in env->r4
        "str r5, [%0, #4]\n"   // Store r5 in env->r5
        "str r6, [%0, #8]\n"   // Store r6 in env->r6
        "str r7, [%0, #12]\n"  // Store r7 in env->r7
        "str r8, [%0, #16]\n"  // Store r8 in env->r8
        "str r9, [%0, #20]\n"  // Store r9 in env->r9
        "str r10, [%0, #24]\n" // Store r10 in env->r10
        "str r11, [%0, #28]\n" // Store r11 in env->r11 (frame pointer)
        "str sp, [%0, #32]\n"  // Store the stack pointer in env->sp
        "str lr, [%0, #36]\n"  // Store the link register in env->lr
        :
        : "r"(&env)
        : "memory"
    );
    return 0;
}

void longjmp(int val) {
    if (val == 0) {
        val = 1;
    }

    asm volatile(
        "ldr r4, [%0]\n"       // Load r4 from env->r4
        "ldr r5, [%0, #4]\n"   // Load r5 from env->r5
        "ldr r6, [%0, #8]\n"   // Load r6 from env->r6
        "ldr r7, [%0, #12]\n"  // Load r7 from env->r7
        "ldr r8, [%0, #16]\n"  // Load r8 from env->r8
        "ldr r9, [%0, #20]\n"  // Load r9 from env->r9
        "ldr r10, [%0, #24]\n" // Load r10 from env->r10
        "ldr r11, [%0, #28]\n" // Load r11 from env->r11 (frame pointer)
        "ldr sp, [%0, #32]\n"  // Load the stack pointer from env->sp
        "ldr lr, [%0, #36]\n"  // Load the link register from env->lr
        :
        : "r"(&env)
        : "memory"
    );
    
    asm volatile("mov r0, %0\n" : : "r"(val)); // Set the return value
    asm volatile("bx lr\n"); // Branch to the link register
    panic("unreachable - jump failed!");
}