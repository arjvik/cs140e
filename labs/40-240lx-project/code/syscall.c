#include <stdint.h>
#include <stdbool.h>
#include "rpi.h"
#include "full-except.h"

//#define SYSCALL(x) asm volatile("swi " #x ::: "memory")
#define SYSCALL0(x) { asm volatile("mov r7, %0; \
                                    swi #128" :: "r"(x) : "memory", "r7"); }
#define SYSCALL1(x, a) { asm volatile("mov r7, %0; \
                                       mov r0, %1; \
                                       swi #128" :: "r"(x), "r"(a) : "r0", "r7", "memory"); }
#define SYSCALL2(x, a, b) { asm volatile("mov r7, %0; \
                                          mov r0, %1; \
                                          mov r1, %2; \
                                          swi #128" :: "r"(x), "r"(a), "r"(b) : "r0", "r1", "r7", "memory"); }
#define SYSCALL3(x, a, b, c) { asm volatile("mov r7, %0; \
                                             mov r0, %1; \
                                             mov r1, %2; \
                                             mov r2, %3; \
                                             swi #128" :: "r"(x), "r"(a), "r"(b), "r"(c) : "r0", "r1", "r2", "r3", "r7", "memory"); }
#define SYSCALL4(x, a, b, c, d) { asm volatile("mov r7, %0; \
                                                mov r0, %1; \
                                                mov r1, %2; \
                                                mov r2, %3; \
                                                mov r3, %4; \
                                                swi #128" :: "r"(x), "r"(a), "r"(b), "r"(c), "r"(d) : "memory", "r0", "r1", "r2", "r3", "r7", "memory"); }

// https://github.com/torvalds/linux/blob/96fca68c4fbf77a8185eb10f7557e23352732ea2/arch/arm/tools/syscall.tbl#L14

static int syscall_handler(regs_t *r) {
    unsigned *pc = (unsigned *)(r->regs[REGS_PC] - 4);
    unsigned sysno = *pc & ((1<<24)-1);

    printk("pc %x, instr %x\n", pc, *pc);
    assert(sysno == 128);
    printk("SWI Interrupt at %x - sysno #%d - from %s\n", pc, sysno, mode_get(cpsr_get()) == USER_MODE ? "user mode" : "super mode");
    if (r->regs[0])
        printk("r0: %x, r1: %x, r2: %x, r3: %x, r7: %x\n", r->regs[0], r->regs[1], r->regs[2], r->regs[3], r->regs[7]);
    else
        printk("r0: %x, r1: %x, r2: %x, r3: %x, r4: %x, r5: %x, r6: %x, r7: %x\n", r->regs[0], r->regs[1], r->regs[2], r->regs[3], r->regs[4], r->regs[5], r->regs[6], r->regs[7]);
    return 0;
}

void notmain() {
    full_except_install(0);
    full_except_set_syscall(syscall_handler);
    
    uint32_t cpsr_cur = cpsr_get();
    assert(mode_get(cpsr_cur) == SUPER_MODE);
    uint32_t cpsr_A = mode_set(cpsr_cur, USER_MODE);

    printk("Hello, world from %s!\n", mode_get(cpsr_A) == USER_MODE ? "user mode" : "super mode");
    SYSCALL0(0);
    printk("Still in %s\n", mode_get(cpsr_A) == USER_MODE ? "user mode" : "super mode");
    SYSCALL1(1, 0x1234);
    printk("Still in %s\n", mode_get(cpsr_A) == USER_MODE ? "user mode" : "super mode");
    SYSCALL2(2, 0x1234, 0x5678);
    printk("Still in %s\n", mode_get(cpsr_A) == USER_MODE ? "user mode" : "super mode");
    SYSCALL3(3, 0x1234, 0x5678, 0x9abc);
    printk("Still in %s\n", mode_get(cpsr_A) == USER_MODE ? "user mode" : "super mode");
    SYSCALL4(4, 0x1234, 0x5678, 0x9abc, 0xdef0);
    printk("Still in %s\n", mode_get(cpsr_A) == USER_MODE ? "user mode" : "super mode");

    asm volatile("mov r0, #0x0000; \
                  mov r1, #0x1100; \
                  mov r2, #0x2200; \
                  mov r3, #0x3300; \
                  mov r4, #0x4400; \
                  mov r5, #0x5500; \
                  mov r6, #0x6600; \
                  mov r7, #0x7700; \
                  swi #128" ::: "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7");
    printk("Still in %s\n", mode_get(cpsr_A) == USER_MODE ? "user mode" : "super mode");
    clean_reboot();
}