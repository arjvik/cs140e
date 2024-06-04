#include <stdbool.h>

struct jmp_buf {
    unsigned int r4;  // Callee-saved register
    unsigned int r5;  // Callee-saved register
    unsigned int r6;  // Callee-saved register
    unsigned int r7;  // Callee-saved register
    unsigned int r8;  // Callee-saved register
    unsigned int r9;  // Callee-saved register
    unsigned int r10; // Callee-saved register
    unsigned int r11; // Callee-saved register (frame pointer)
    unsigned int sp;  // Stack pointer
    unsigned int lr;  // Link register
};

extern struct jmp_buf env;
extern bool jmp_set;

void longjmp(int val) __attribute__((noreturn));
int setjmp();