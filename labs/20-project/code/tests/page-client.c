#include <stdint.h>
#include "rpi.h"
#include "parallel.h"

#define ONE_MB (1024*1024)

void notmain() {

    unsigned start = timer_get_usec();

    parallel_setup_write();
    
    printk("TWO: Ready to send.\n");

    parallel_write_32(0x9100000);

    parallel_setup_read();

    printk("TWO: Ready to receive.\n");

    parallel_read_n((uint32_t *) 0x9100000, ONE_MB);

    unsigned end = timer_get_usec();
    printk("TWO: Took %d\n", (end - start) / 1000);

    clean_reboot();
}