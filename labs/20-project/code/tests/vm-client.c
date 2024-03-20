#include <stdint.h>
#include "rpi.h"
#include "parallel.h"
#include "full-except.h"
#include "../../16-vm-page-table/code/pt-vm.h"
#include "../../16-vm-page-table/code/mem-attr.h"

#define ONE_MB (1024*1024)

vm_pt_t *pt;
pin_t dev, kern;

void test(void) __attribute__((section(".extratext")));
void test(void) {
    printk("hello from test\n");
}

static void fault_handler(regs_t *r) {
    uint32_t fault_addr;
    uint32_t dfsr;

    // b4-44
    asm volatile("MRC p15, 0, %0, c6, c0, 0" : "=r" (fault_addr));

    // ~3-66
    asm volatile("MRC p15, 0, %0, c5, c0, 0" : "=r" (dfsr));

    printk("Data fault on address=%x\n", fault_addr);


    vm_map_sec(pt, 2*ONE_MB, 2*ONE_MB, kern);
    staff_mmu_sync_pte_mods();
}

void notmain() {

    test();

    full_except_install(0);
    full_except_set_data_abort(fault_handler);

    kmalloc_init();
    pt = kmalloc_aligned(4096*4, 1<<14);
    mmu_init();
    domain_access_ctrl_set(~0);

    dev  = pin_mk_global(0, 0b011, MEM_device);
    kern = pin_mk_global(0, 0b011, MEM_uncached);
        
    // all the device memory: identity map
    vm_map_sec(pt, 0x20000000, 0x20000000, dev);
    vm_map_sec(pt, 0x20100000, 0x20100000, dev);
    vm_map_sec(pt, 0x20200000, 0x20200000, dev);
    // map first two MB for the kernel
    vm_map_sec(pt, 0, 0, kern);
    vm_map_sec(pt, ONE_MB, ONE_MB, kern);
    // kernel stack, interrupt stack
    vm_map_sec(pt, STACK_ADDR - ONE_MB, STACK_ADDR - ONE_MB, kern);
    vm_map_sec(pt, INT_STACK_ADDR - ONE_MB, INT_STACK_ADDR - ONE_MB, kern);

    vm_mmu_switch(pt, 0x140e, 1);
    vm_mmu_enable();

    assert(mmu_is_enabled());
    trace("MMU is on and working!\n");
    PUT32(2*ONE_MB, 0x12345678);
    trace("GET32(%x)=%x\n", 2*ONE_MB, GET32(2*ONE_MB));

    test();
}