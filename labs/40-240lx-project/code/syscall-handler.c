#include <stdio.h>
#include "rpi.h"
#include "syscalls-common.h"
#include "syscall-handler.h"

int sys_exit(int status) {
    if (status != 0)
        printk("Exiting with nonzero status %d! (forcing clean exit anyway)\n", status);
    clean_reboot();
}

int sys_write(int fd, const void *buf, size_t count) {
    if (fd != 1 && fd != 2)
        panic("Read-only filesystem!\n");
    // for (size_t i = 0; i < count; i++)
        // uart_put8(((char *)buf)[i]);
    printk("%s", buf);
    return 1;
}

int syscall_handler(regs_t *r) {
    unsigned *pc = (unsigned *)(r->regs[REGS_PC] - 4);
    unsigned immediate = *pc & ((1<<24)-1);
    assert(immediate == 0);
    unsigned sysno = r->regs[7];

    // assert(sysno == 0); // stupid GCC, should be 128
    printk("        (handling syscall %d, r0=%x r1=%x r2=%x)\n", sysno, r->regs[0], r->regs[1], r->regs[2]);
    
    int (*sys_handlers[])() = {
        [SYS_EXIT] = sys_exit,
        [SYS_WRITE] = sys_write,
    };

    if (!sys_handlers[sysno])
        panic("Unknown syscall %d\n", sysno);

    return sys_handlers[sysno](r->regs[0], r->regs[1], r->regs[2]);
}