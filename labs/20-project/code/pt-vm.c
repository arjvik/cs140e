#include "rpi.h"
#include "pt-vm.h"
#include "helper-macros.h"
#include "procmap.h"

// turn this off if you don't want all the debug output.
enum { verbose_p = 1 };
enum { OneMB = 1024*1024 };

vm_pt_t *vm_pt_alloc(unsigned n) {
    demand(n == 4096, we only handling a fully-populated page table right now);

    vm_pt_t *pt = 0;
    unsigned nbytes = 4096 * sizeof *pt;

    // allocate pt with n entries.
    // pt = staff_vm_pt_alloc(n);
    pt = kmalloc_aligned(nbytes, 1<<14);

    demand(is_aligned_ptr(pt, 1<<14), must be 14-bit aligned!);
    return pt;
}

// allocate new page table and copy pt.
vm_pt_t *vm_dup(vm_pt_t *pt1) {
    vm_pt_t *pt2 = vm_pt_alloc(PT_LEVEL1_N);
    memcpy(pt2,pt1,PT_LEVEL1_N * sizeof *pt1);
    return pt2;
}

// same as pinned version: probably should check that
// the page table is set.
void vm_mmu_enable(void) {
    assert(!mmu_is_enabled());
    mmu_enable();
    assert(mmu_is_enabled());
}

// same as pinned version.
void vm_mmu_disable(void) {
    assert(mmu_is_enabled());
    mmu_disable();
    assert(!mmu_is_enabled());
}

// - set <pt,pid,asid> for an address space.
// - must be done before you switch into it!
// - mmu can be off or on.
void vm_mmu_switch(vm_pt_t *pt, uint32_t pid, uint32_t asid) {
    assert(pt);
    mmu_set_ctx(pid, asid, pt);
}

// just like pinned.
void vm_mmu_init(uint32_t domain_reg) {
    // initialize everything, after bootup.
    mmu_init();
    domain_access_ctrl_set(domain_reg);
}

// map the 1mb section starting at <va> to <pa>
// with memory attribute <attr>.
vm_pte_t *
vm_map_sec(vm_pt_t *pt, uint32_t va, uint32_t pa, pin_t attr) 
{
    assert(aligned(va, OneMB));
    assert(aligned(pa, OneMB));

    // today we just use 1mb.
    assert(attr.pagesize == PAGE_1MB);

    unsigned index = va >> 20;
    assert(index < PT_LEVEL1_N);

    vm_pte_t *pte = &pt[index];
    // return staff_vm_map_sec(pt,va,pa,attr);

    *pte = (vm_pte_t) {
        .tag = 0b10,
        .B = attr.mem_attr & 0b1,
        .C = (attr.mem_attr >> 1) & 0b1,
        .XN = 0,
        .domain = attr.dom,
        .IMP = 0,
        .AP = attr.AP_perm & 0b11,
        .TEX = attr.mem_attr >> 2,
        .APX = attr.AP_perm >> 2,
        .S = 0,
        .nG = !attr.G,
        .super = 0,
        ._sbz1 = 0,
        .sec_base_addr = pa >> 20
    };

    if(verbose_p)
        // vm_pte_print(pt,pte);
    assert(pte);
    return pte;
}

// lookup 32-bit address va in pt and return the pte
// if it exists, 0 otherwise.
vm_pte_t *vm_lookup(vm_pt_t *pt, uint32_t va) {
    // return staff_vm_lookup(pt,va);
    vm_pte_t *x = &pt[va >> 20];
    return x->tag ? x : 0;
}

// manually translate <va> in page table <pt>
// - if doesn't exist, returns 0.
// - if does exist returns the corresponding physical
//   address in <pa>
//
// NOTE: we can't just return the result b/c page 0 could be mapped.
vm_pte_t *vm_xlate(uint32_t *pa, vm_pt_t *pt, uint32_t va) {
    // return staff_vm_xlate(pa,pt,va);
    vm_pte_t *x = &pt[va >> 20];
    if (x->tag) {
        *pa = (x->sec_base_addr << 20) | (va & ((1 << 20) - 1));
        return x;
    }
    return 0;
}

// compute the default attribute for each type.
static inline pin_t attr_mk(pr_ent_t *e) {
    switch(e->type) {
    case MEM_DEVICE: 
        return pin_mk_device(e->dom);
    // kernel: currently everything is uncached.
    case MEM_RW:
        return pin_mk_global(e->dom, perm_rw_priv, MEM_uncached);
   case MEM_RO: 
        panic("not handling\n");
   default: 
        panic("unknown type: %d\n", e->type);
    }
}

// setup the initial kernel mapping.  This will mirror
//  static inline void procmap_pin_on(procmap_t *p) 
// in <15-pinned-vm/code/procmap.h>  but will call
// your vm_ routines, not pinned routines.
//
// if <enable_p>=1, should enable the MMU.  make sure
// you setup the page table and asid. use  
// kern_asid, and kern_pid.
vm_pt_t *vm_map_kernel(procmap_t *p, int enable_p) {
    // the asid and pid we start with.  
    //    shouldn't matter since kernel is global.
    enum { kern_asid = 1, kern_pid = 0x140e };

    vm_mmu_init(dom_perm(p->dom_ids, DOM_client));
    
    vm_pt_t *pt = vm_pt_alloc(PT_LEVEL1_N);
    
    for (unsigned i = 0; i < p->n; i++) {
        pr_ent_t *e = &p->map[i];
        demand(e->nbytes == OneMB, "nbytes=%d\n", e->nbytes);
        pin_t g = attr_mk(e);
        vm_map_sec(pt, e->addr, e->addr, g);
    }

    vm_mmu_switch(pt, kern_pid, kern_asid);
    // mmu_set_ctx(kern_pid, kern_asid, pt); // does the same thing
    
    if (enable_p)
        mmu_enable();
    // return staff_vm_map_kernel(p,enable_p);

    assert(pt);
    return pt;
}

// static inline void procmap_pin(procmap_t *p) {
//     for(unsigned i = 0; i < p->n; i++) {
//         pr_ent_t *e = &p->map[i];
//         demand(e->nbytes == MB, "nbytes=%d\n", e->nbytes);

//         pin_t g;
//         switch(e->type) {
//         case MEM_DEVICE:
//             g = pin_mk_device(e->dom);
//             break;
//         case MEM_RW:
//             // currently everything is uncached.
//             g = pin_mk_global(e->dom, perm_rw_priv, MEM_uncached);
//             break;
//         case MEM_RO: panic("not handling\n");
//         default: panic("unknown type: %d\n", e->type);
//         }
//         pin_mmu_sec(i, e->addr, e->addr, g);
//     }
// }

// static inline void procmap_pin_on(procmap_t *p) {
//     // compute all the domain permissions.
//     uint32_t d = dom_perm(p->dom_ids, DOM_client);

//     pin_mmu_init(d);
//     procmap_pin(p);

//     pin_debug("about to turn on mmu\n");

//     // setup.
//     pin_mmu_switch(0,1);
//     pin_mmu_enable();

//     assert(mmu_is_enabled());
//     pin_debug("enabled!\n");

//     // can only check this after MMU is on.
//     pin_debug("going to check entries are pinned\n");
//     for(unsigned i = 0; i < p->n; i++)
//         pin_check_exists(p->map[i].addr);
// }