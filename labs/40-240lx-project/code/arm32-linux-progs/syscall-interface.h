static inline int syscall1(int sysno, int arg1) {
    int result;
    asm volatile (
        "mov r7, %1 \n"
        "mov r0, %2 \n"
        "swi 0 \n"
        "mov %0, r0 \n"
        : "=r" (result)
        : "r" (sysno), "r" (arg1)
        : "r7", "r0"
    );
    return result;
}

static inline int syscall2(int sysno, int arg1, int arg2) {
    int result;
    asm volatile (
        "mov r7, %1 \n"
        "mov r0, %2 \n"
        "mov r1, %3 \n"
        "swi 0 \n"
        "mov %0, r0 \n"
        : "=r" (result)
        : "r" (sysno), "r" (arg1), "r" (arg2)
        : "r7", "r0", "r1"
    );
    return result;
}

static inline int syscall3(int sysno, int arg1, int arg2, int arg3) {
    int result;
    asm volatile (
        "mov r7, %1 \n"
        "mov r0, %2 \n"
        "mov r1, %3 \n"
        "mov r2, %4 \n"
        "swi 0 \n"
        "mov %0, r0 \n"
        : "=r" (result)
        : "r" (sysno), "r" (arg1), "r" (arg2), "r" (arg3)
        : "r7", "r0", "r1", "r2"
    );
    return result;
}