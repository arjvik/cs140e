// check if a register is caller or callee saved by automatically
// checking the machine code
//
// you need to implement <todo> and handle all registers from r0-r12
#include "rpi.h"

#define STRING(x) #x

// write this: should return 1 if it's caller saved.
// hint: what machine instruction should be at the 
// function's address?

// given a routine, return 1 if <fp> points to an empty
// routine that just returns.
//
// we use this to tell if a register <r> is caller saved:
//   1. generate a routine that only clobbers <r>
//   2. if routine is empty, <r> was a caller reg.
static inline int is_empty(void (*fp)(void)) {
    return *(uint32_t*)fp == 0xe12fff1e; // bx lr
}

// generates a function that has a single inline assembly 
// statement that clobbers the given register
// [see the caller-callee note]
#define clobber_reg_gen(reg)                    \
    static void clobber_ ## reg(void) {         \
        asm volatile("" : : : STRING(reg));     \
    }                       

#define assert_caller(reg) do {                                 \
    if(!is_empty(clobber_ ## reg))                              \
        panic("reg %s is not caller\n", STRING(reg));           \
    else                                                        \
        trace("expected: reg %s is caller\n", STRING(reg));     \
} while(0)

#define assert_callee(reg) do {                                 \
    if(is_empty(clobber_ ## reg))                               \
        panic("reg %s is not callee\n", STRING(reg));           \
    else                                                        \
        trace("expected: reg %s is callee\n", STRING(reg));     \
} while(0)


// generate the clobber routines.
clobber_reg_gen(r0)
clobber_reg_gen(r1)
clobber_reg_gen(r2)
clobber_reg_gen(r3)
clobber_reg_gen(r4)
clobber_reg_gen(r5)
clobber_reg_gen(r6)
clobber_reg_gen(r7)
clobber_reg_gen(r8)
clobber_reg_gen(r9)
clobber_reg_gen(r10)
clobber_reg_gen(r11)
clobber_reg_gen(r12)
clobber_reg_gen(r13)
clobber_reg_gen(r14)



// FILL this in with all caller saved registers.
// these are all the registers you *DO NOT* during voluntary
// context switching 
//
// NOTE: ignore r13,r14,r15
void check_cswitch_ignore_regs(void) {
    assert_caller(r0);
    assert_caller(r1);
    assert_caller(r2);
    assert_caller(r3);
    assert_caller(r12);
    // r13 is the stack pointer, so it's not caller or callee saved.
    // assert_caller(r13);
    // r15 is the pc, so it's not caller or callee saved.
    // assert_caller(r15);

    // if you reach here it passed.
    trace("ignore regs passed\n");
}

// put all the regs you do save during context switching
// here [callee saved] 
//
// NOTE: ignore r13,r14,r15
void check_cswitch_save_regs(void) {
    assert_callee(r4);
    assert_callee(r5);
    assert_callee(r6);
    assert_callee(r7);
    assert_callee(r8);
    assert_callee(r9);
    assert_callee(r10);
    assert_callee(r11);
    assert_callee(r14);
    trace("saved regs passed\n");
}

void notmain() {
    check_cswitch_ignore_regs();
    check_cswitch_save_regs();
    trace("SUCCESS\n");
}
