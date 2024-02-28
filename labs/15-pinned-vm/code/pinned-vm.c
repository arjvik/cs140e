// put your code here.
//
#include "rpi.h"
#include "libc/bit-support.h"

// has useful enums and helpers.
#include "vector-base.h"
#include "pinned-vm.h"
#include "mmu.h"
#include "procmap.h"

// generate the _get and _set methods.
// (see asm-helpers.h for the cp_asm macro 
// definition)
// arm1176.pdf: 3-149

// define the following routines.
#if 1
// arm1176.pdf: 3-149
static void lockdown_index_set(uint32_t x);
static uint32_t lockdown_index_get(void);
cp_asm(lockdown_index, p15, 5, c15, c4, 2)

static void lockdown_va_set(uint32_t x);
static uint32_t lockdown_va_get(void);
cp_asm(lockdown_va, p15, 5, c15, c5, 2)

static void lockdown_pa_set(uint32_t x);
static uint32_t lockdown_pa_get(void);
cp_asm(lockdown_pa, p15, 5, c15, c6, 2)

static void lockdown_attr_set(uint32_t x);
static uint32_t lockdown_attr_get(void);
cp_asm(lockdown_attr, p15, 5, c15, c7, 2)

void xlate_pa_set(uint32_t x);

// routines to manually check that a translation
// can succeed.  we use these to check that 
// pinned translations are in the TLB.
// see:
//    p 3-80---3-82 in arm1176.pdf

// translate for a privileged read access
static void xlate_kern_rd_set(uint32_t x);
// MCR p15,0,<Rn>,c7,c8,3
cp_asm_set(xlate_kern_rd, p15, 0, c7, c8, 1)

// translate for a priviledged write access
void xlate_kern_wr_set(uint32_t x);

// get physical address after manual translation
static uint32_t xlate_pa_get(void);
// MRC p15,0,<Rd>,c7,c4,0
cp_asm_get(xlate_pa, p15, 0, c7, c4, 0)


#endif

// do a manual translation in tlb:
//   1. store result in <result>
//   2. return 1 if entry exists, 0 otherwise.
int tlb_contains_va(uint32_t *result, uint32_t va) {
    // 3-79
    assert(bits_get(va, 0,2) == 0);
    // return staff_tlb_contains_va(result, va);
    xlate_kern_rd_set(va);
    *result = xlate_pa_get();
    if (*result & 0x1) {
        return 0;
    } else {
        *result = *result & ~((1 << 10) - 1);
        return 1;
    }
    // return 0;
}

// map <va>-><pa> at TLB index <idx> with attributes <e>
void pin_mmu_sec(unsigned idx,  
                uint32_t va, 
                uint32_t pa,
                pin_t e) {

    // staff_pin_mmu_sec(idx, va, pa, e);
    // return;

    demand(idx < 8, lockdown index too large);
    // lower 20 bits should be 0.
    demand(bits_get(va, 0, 19) == 0, only handling 1MB sections);
    demand(bits_get(pa, 0, 19) == 0, only handling 1MB sections);

    // if(va != pa)
    //     panic("for today's lab, va (%x) should equal pa (%x)\n",
    //             va,pa);

    debug("about to map %x->%x\n", va,pa);


    // these will hold the values you assign for the tlb entries.
    uint32_t x, va_ent, pa_ent, attr;
    x = idx;
    va_ent = (va | (e.G ? 1 << 9 : 0) | (e.asid & ((1 << 8) - 1)));
    attr = ((e.dom & ((1 << 4) - 1)) << 7 | e.mem_attr << 1);
    pa_ent = (pa | e.pagesize << 6 | e.AP_perm << 1 | 1);
    // put your code here.
    lockdown_index_set(x);
    lockdown_va_set(va_ent);
    lockdown_attr_set(attr); 
    lockdown_pa_set(pa_ent);


#if 1
    // put this back in when defined.
    if((x = lockdown_va_get()) != va_ent)
        panic("lockdown va: expected %x, have %x\n", va_ent,x);
    if((x = lockdown_pa_get()) != pa_ent)
        panic("lockdown pa: expected %x, have %x\n", pa_ent,x);
    if((x = lockdown_attr_get()) != attr)
        panic("lockdown attr: expected %x, have %x\n", attr,x);
#endif
}

// check that <va> is pinned.  
void pin_check_exists(uint32_t va) {
    if(!mmu_is_enabled())
        panic("XXX: i think we can only check existence w/ mmu enabled\n");

    uint32_t r;
    if(tlb_contains_va(&r, va)) {
        pin_debug("success: TLB contains %x, returned %x\n", va, r);
        assert(va == r);
    } else
        panic("TLB should have %x: returned %x [reason=%b]\n", 
            va, r, bits_get(r,1,6));
}

void domain_access_ctrl_set(uint32_t d) {
    staff_domain_access_ctrl_set(d);
}

static void *null_pt = 0;

// turn the pinned MMU system on.
//    1. initialize the MMU (maybe not actually needed): clear TLB, caches
//       etc.  if you're obsessed with low line count this might not actually
//       be needed, but we don't risk it.
//    2. allocate a 2^14 aligned, 0-filled 4k page table so that any nonTLB
//       access gets a fault.
//    3. set the domain privileges (to DOM_client)
//    4. set the exception handler up using <vector_base_set>
//    5. turn the MMU on --- this can be much simpler than the normal
//       mmu procedure since it's never been on yet and we do not turn 
//       it off.
//    6. profit!

// fill this in based on the test code.
void pin_mmu_init(uint32_t domain_reg) {
    staff_mmu_init();
    null_pt = kmalloc_aligned(4096*4, 1<<14);
    domain_access_ctrl_set(domain_reg);
}

void pin_mmu_switch(uint32_t pid, uint32_t asid) {
    assert(null_pt);
    staff_mmu_set_ctx(pid, asid, null_pt);
}

void lockdown_print_entry(unsigned idx) {
    trace("   idx=%d\n", idx);
    lockdown_index_set(idx);
    uint32_t va_ent = lockdown_va_get();
    uint32_t pa_ent = lockdown_pa_get();
    unsigned v = bit_get(pa_ent, 0);

    if(!v) {
        trace("     [invalid entry %d]\n", idx);
        return;
    }

    // 3-149
    uint32_t va = (va_ent & ~((1 << 12) - 1)) >> 12;
    uint32_t G = !!(va_ent & (1 << 9));
    uint32_t asid = va_ent & ((1 << 8) - 1);
    trace("     va_ent=%x: va=%x|G=%d|ASID=%d\n",
        va_ent, va, G, asid);

    // 3-150
    uint32_t pa = (pa_ent & ~((1 << 12) - 1)) >> 12;
    uint32_t nsa = !!(pa_ent & (1 << 9));
    uint32_t nstid = !!(pa_ent & (1 << 8));
    uint32_t size = (pa_ent >> 6) & 0b11;
    uint32_t apx = (pa_ent >> 1) & 0b111;
    trace("     pa_ent=%x: pa=%x|nsa=%d|nstid=%d|size=%b|apx=%b|v=%d\n",
                pa_ent, pa, nsa,nstid,size, apx,v);

    // 3-151
    uint32_t attr = lockdown_attr_get();
    uint32_t dom = (attr >> 7) & 0b1111;
    uint32_t xn = !!(attr & (1 << 6));
    uint32_t tex = (attr >> 3) & 0b111;
    uint32_t C = !!(attr & (1 << 2));
    uint32_t B = !!(attr & (1 << 1));
    trace("     attr=%x: dom=%d|xn=%d|tex=%b|C=%d|B=%d\n",
            attr, dom,xn,tex,C,B);
}

void lockdown_print_entries(const char *msg) {
    trace("-----  <%s> ----- \n", msg);
        trace("  pinned TLB lockdown entries:\n");
        for(int i = 0; i < 8; i++)
            lockdown_print_entry(i);
        trace("----- ---------------------------------- \n");
    }
