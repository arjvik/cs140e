#include <stdint.h>
#include <stdbool.h>
#include "rpi.h"
#include "parallel.h"
#include "full-except.h"
#include "pt-vm.h"
#include "mem-attr.h"
#include "vm-4k-pages.h"
#include "fat32.h"

#define ONE_MB (1024*1024)
#define FOUR_K (4*1024)

vm_pt_t *pt;
pin_t dev, kern, kern4k;

static void fault_handler(regs_t *r) {
    uint32_t fault_addr;

    asm volatile("MRC p15, 0, %0, c6, c0, 0" : "=r" (fault_addr));

    panic("Data fault on address=%x\n", fault_addr);
}

static void prefetch_fault_handler(regs_t *r) {
    uint32_t fault_addr;

    asm volatile("MRC p15, 0, %0, c6, c0, 0" : "=r" (fault_addr));

    panic("Instruction fault on address=%x\n", fault_addr);
}

static int syscall_handler(regs_t *r) {
    printk("SWI Interrupt at %x\n", r->regs[REGS_PC] - 4);

    r->regs[13] += sizeof(*r);
    r->regs[14] = (uint32_t) clean_reboot;

    printk("r0: %x ", r->regs[0]);
    printk("r1: %x ", r->regs[1]);
    printk("r2: %x ", r->regs[2]);
    printk("r3: %x\n", r->regs[3]);
    printk("r4: %x ", r->regs[4]);
    printk("r5: %x", r->regs[5]);
    printk("r6: %x", r->regs[6]);
    printk("r7: %x\n", r->regs[7]);
    printk("r8: %x ", r->regs[8]);
    printk("r9: %x ", r->regs[9]);
    printk("r10: %x ", r->regs[10]);
    printk("r11: %x\n", r->regs[11]);
    printk("r12: %x ", r->regs[12]);
    printk("sp: %x ", r->regs[13]);
    printk("lr: %x ", r->regs[14]);
    printk("pc: %x\n", r->regs[15]);
    
    parallel_setup_write();
    parallel_write_n(r->regs, sizeof(*r));

    asm volatile("add sp, sp, #68");

    printk("Ready.\n");
    while (true) {
        parallel_setup_read();
        uint32_t *addr = (uint32_t *) parallel_read_32();
        printk("Sending %x over the wire...", addr);
        parallel_setup_write();
        parallel_write_n(addr, FOUR_K);
        printk("done.\n");
    }
    return 0;
}

void notmain() {
    full_except_install(0);
    full_except_set_data_abort(fault_handler);
    full_except_set_prefetch(prefetch_fault_handler);
    full_except_set_syscall(syscall_handler);

    kmalloc_init();
    pt = kmalloc_aligned(4096*4, 1<<14);
    mmu_init();
    domain_access_ctrl_set(~0);

    dev    = pin_mk_global(0, 0b011, MEM_device);
    kern   = pin_mk_global(0, 0b011, MEM_uncached);
    kern4k = pin_mk_global_4k(0, 0b011, MEM_uncached);
        
    // all the device memory: identity map
    vm_map_sec(pt, 0x20000000, 0x20000000, dev);
    vm_map_sec(pt, 0x20100000, 0x20100000, dev);
    vm_map_sec(pt, 0x20200000, 0x20200000, dev);
    vm_map_sec(pt, 0x20300000, 0x20300000, dev);
    // map first two MB for the kernel
    vm_map_sec(pt, 0, 0, kern);
    // for (unsigned i = 0; i < 0xe000; i += FOUR_K)
    //     vm_map_sec_4k(pt, i, i, kern4k);
    vm_map_sec(pt, ONE_MB, ONE_MB, kern); // kmalloc heap, page tables
    vm_map_sec_4k(pt, 2*ONE_MB, 2*ONE_MB, kern4k);
    // kernel stack, interrupt stack
    vm_map_sec(pt, STACK_ADDR - ONE_MB, STACK_ADDR - ONE_MB, kern);
    vm_map_sec(pt, INT_STACK_ADDR - ONE_MB, INT_STACK_ADDR - ONE_MB, kern);

    vm_mmu_switch(pt, 0x140e, 1);
    vm_mmu_enable();

    assert(mmu_is_enabled());
    trace("MMU is on and working!\n");

    // test();

    vm_map_sec(pt, 0x900000, 0x900000, kern);
    staff_mmu_sync_pte_mods();

    fat32init();
    struct fat32_directory_entry binary;
    assert(findFile("hello-f.bin", 2, &binary));
    readEntireFile((uint8_t *) 0x900000, binary);

    printk("Loaded hello-f.bin!\n");

    // Exec hello-f.bin
    ((void (*)(void)) 0x900000)();
}