#ifndef __VM_4K_PAGES_H__
#define __VM_4K_PAGES_H__

#include "rpi.h"
#include "pt-vm.h"
#include "helper-macros.h"
#include "procmap.h"
#include "mem-attr.h"

typedef struct first_level_descriptor_coarse_table {
    unsigned
        tag : 2,      // 0-1:2, 0b01,
        _sbz1 : 3,    // 2-4:3, bit3 = NS
        domain : 4,   // 5-8:4
        P : 1,        // 9:1
        coarse_table_base_addr : 22; // 10-31:22
} fld_coarse_t;
_Static_assert(sizeof(fld_coarse_t) == 4, "invalid size for fld_coarse_t!");

typedef struct second_level_descriptor {
    unsigned
        XN : 1,      // 0:1
        tag : 1 ,    // 1:1, 0b1
        B : 1,       // 2:1
        C : 1,       // 3:1
        AP : 2,      // 4-5:2
        TEX : 3,     // 6-8:3
        APX : 1,     // 9:1
        S : 1,       // 10:1
        nG : 1,      // 11:1
        small_base_addr : 20; // 12-31:20
} sld_t;
_Static_assert(sizeof(sld_t) == 4, "invalid size for fld_t!");

sld_t *
vm_map_sec_4k(vm_pt_t *pt, uint32_t va, uint32_t pa, pin_t attr);

static inline pin_t
pin_mk_global_4k(uint32_t dom, mem_perm_t perm, mem_attr_t attr) {
    // G=1, asid=0.
    pin_t p = pin_mk(1, dom, 0, perm, attr);
    p.pagesize = PAGE_4K;
    return p;
}

#endif