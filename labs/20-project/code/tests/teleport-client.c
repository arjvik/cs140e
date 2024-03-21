#include <stdint.h>
#include "rpi.h"
#include "parallel.h"
#include "full-except.h"
#include "pt-vm.h"
#include "mem-attr.h"
#include "vm-4k-pages.h"

#define ONE_MB (1024*1024)
#define FOUR_K (4*1024)

vm_pt_t *pt;
pin_t dev, kern, kern4k;

static void fault_handler(regs_t *r) {
    uint32_t fault_addr;
    // uint32_t dfsr;

    // b4-44
    asm volatile("MRC p15, 0, %0, c6, c0, 0" : "=r" (fault_addr));

    // // ~3-66
    // asm volatile("MRC p15, 0, %0, c5, c0, 0" : "=r" (dfsr));

    printk("Data fault on address=%x\n", fault_addr);


    // vm_map_sec(pt, 2*ONE_MB, 2*ONE_MB, kern);
    vm_map_sec_4k(pt, 2*ONE_MB+FOUR_K, 2*ONE_MB+FOUR_K, kern4k);
    staff_mmu_sync_pte_mods();
}

static void prefetch_fault_handler(regs_t *r) {
    uint32_t fault_addr;
    uint32_t dfsr;

    // b4-44
    asm volatile("MRC p15, 0, %0, c6, c0, 2" : "=r" (fault_addr));

    printk("Instruction fault on address=%x\n", fault_addr);

    uint32_t *page_base = (uint32_t *) (fault_addr & ~(((uint32_t) FOUR_K) - 1));
    vm_map_sec_4k(pt, (uint32_t) page_base, (uint32_t) page_base, kern4k);
    staff_mmu_sync_pte_mods();

    parallel_setup_write();
    parallel_write_32((uint32_t) page_base);
    parallel_setup_read();
    parallel_read_n(page_base, FOUR_K);
}

void notmain() {
    full_except_install(0);
    full_except_set_data_abort(fault_handler);
    full_except_set_prefetch(prefetch_fault_handler);

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
    // void (*mytest)(void) = (void*)0xe000;
    // mytest();
    // BRANCHTO(0xe000);
    // ((void (*)(void)) 0x900000)();
    parallel_setup_read();
    regs_t r;
    parallel_read_n(r.regs, sizeof(r));
    switchto(&r);
}