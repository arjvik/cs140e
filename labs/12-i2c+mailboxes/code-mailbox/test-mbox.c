#include "rpi.h"
#include "mbox.h"

uint32_t rpi_temp_get(void) ;

#include "cycle-count.h"

// compute cycles per second using
//  - cycle_cnt_read();
//  - timer_get_usec();
unsigned cyc_per_sec(void) {
    unsigned start = timer_get_usec();
    unsigned start_cyc = cycle_cnt_read();

    while(timer_get_usec() - start < 1000000)
        ;

    unsigned end = timer_get_usec();
    unsigned end_cyc = cycle_cnt_read();

    unsigned cyc_per_sec = (end_cyc - start_cyc); // division literally doesn't exist?? so we assume exactly 1 second has passed
    // cyc_per_sec = cyc_per_sec * 1000000 / (end - start);
    output("cyc/sec=%d\n", cyc_per_sec);
    return cyc_per_sec;
}

void notmain(void) { 
    output("mailbox serial number = %llx\n", rpi_get_serialnum());

    output("mailbox revision number = %x\n", rpi_get_revision());
    output("mailbox model number = %x\n", rpi_get_model());

    uint32_t size = rpi_get_memsize();
    output("mailbox physical mem: size=%d (%dMB)\n", 
            size, 
            size/(1024*1024));

    // print as fahrenheit
    unsigned x = rpi_temp_get();

    // convert <x> to C and F
    unsigned C = 0, F = 0;
    C = x / 1000;
    F = (C * 9/5) + 32;
    output("mailbox temp = %x, C=%d F=%d\n", x, C, F); 

    // todo("do overclocking!\n");
}
