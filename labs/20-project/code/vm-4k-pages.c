#include "rpi.h"
#include "helper-macros.h"
#include "procmap.h"
#include "vm-4k-pages.h"

enum { OneMB = 1024*1024, FourK = 4*1024 };

sld_t *
vm_map_sec_4k(vm_pt_t *pt, uint32_t va, uint32_t pa, pin_t attr) 
{
    assert(aligned(va, FourK));
    assert(aligned(pa, FourK));

    // today we just use 1mb.
    assert(attr.pagesize == PAGE_4K);

    unsigned index = va >> 20;
    assert(index < PT_LEVEL1_N);

    vm_pte_t *pte = &pt[index];

    assert(pte->tag == 0b00 || pte->tag == 0b01);

    if (pte->tag == 0b00) {
        // allocate a new second level page table.
        *(fld_coarse_t *)pte = (fld_coarse_t) {
            .tag = 0b01,
            ._sbz1 = 0,
            .domain = attr.dom,
            .P = 0,
            .coarse_table_base_addr = ((uint32_t) kmalloc_aligned(4 * 1 << 8, 1<<10)) >> 10
        };
    }
    sld_t *sld_table = (sld_t *) (((fld_coarse_t *)pte)->coarse_table_base_addr << 10);
    sld_t *sld = &sld_table[(va >> 12) & 0xff];
    *sld = (sld_t) {
        .XN = 0,
        .tag = 0b1,
        .B = attr.mem_attr & 0b1,
        .C = (attr.mem_attr >> 1) & 0b1,
        .AP = attr.AP_perm & 0b11,
        .TEX = attr.mem_attr >> 2,
        .APX = attr.AP_perm >> 2,
        .S = 0,
        .nG = !attr.G,
        .small_base_addr = pa >> 12
    };
    return sld;
}