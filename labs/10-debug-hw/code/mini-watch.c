// very dumb, simple interface to wrap up watchpoints better.
// only handles a single watchpoint.
//
// You should be able to take most of the code from the 
// <1-watchpt-test.c> test case where you already did 
// all the needed operations.  This interface just packages 
// them up a bit.
//
// possible extensions:
//   - do as many as hardware supports, perhaps a handler for 
//     each one.
//   - make fast.
//   - support multiple independent watchpoints so you'd return
//     some kind of structure and pass it in as a parameter to
//     the routines.
#include "rpi.h"
#include "mini-watch.h"

// we have a single handler: so just use globals.
static watch_handler_t watchpt_handler = 0;
static void *watchpt_data = 0;

// is it a load fault?
static int mini_watch_load_fault(void) {
    return datafault_from_ld();
}

// if we have a dataabort fault, call the watchpoint
// handler.
static void watchpt_fault(regs_t *r) {
    // watchpt handler.
    if(was_brkpt_fault())
        panic("should only get debug faults!\n");
    if(!was_watchpt_fault())
        panic("should only get watchpoint faults!\n");
    if(!watchpt_handler)
        panic("watchpoint fault without a fault handler\n");

    watch_fault_t w = watch_fault_mk(watchpt_fault_pc(), (unsigned *) cp15_far_get(), mini_watch_load_fault(), r);
    watchpt_handler(watchpt_data, &w);

    // todo("setup the <watch_fault_t> structure");
    // todo("call: watchpt_handler(watchpt_data, &w);");

    // in case they change the regs.
    switchto(w.regs);
}

// setup:
//   - exception handlers, 
//   - cp14, 
//   - setup the watchpoint handler
// (see: <1-watchpt-test.c>
void mini_watch_init(watch_handler_t h, void *data) {
    // todo("setup cp14 and the full exception routines");
    full_except_install(0);
    full_except_set_data_abort(watchpt_fault);
    cp14_enable();

    // just started, should not be enabled.
    assert(!cp14_bcr0_is_enabled());
    assert(!cp14_bcr0_is_enabled());

    watchpt_handler = h;
    watchpt_data = data;
}

// set a watch-point on <addr>.
void mini_watch_addr(void *addr) {
    uint32_t b = 0x1F | 1 << (5 + ((unsigned) addr) % 4);
    cp14_wcr0_set(b);
    cp14_wvr0_set((unsigned) addr & ~0x3);
}

// disable current watchpoint <addr>
void mini_watch_disable(void *addr) {
    cp14_wcr0_disable();
}

// return 1 if enabled.
int mini_watch_enabled(void) {
    return cp14_wcr0_is_enabled();
}

// called from exception handler: if the current 
// fault is a watchpoint, return 1
int mini_watch_is_fault(void) { 
    return was_watchpt_fault();
}
