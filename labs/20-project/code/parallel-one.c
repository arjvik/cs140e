#include <stdint.h>
#include "rpi.h"
#include "parallel.h"

uint8_t msg[] = {0x1D, 0x2E, 0x3A, 0x4D, 0x5B, 0x6E, 0x7E, 0x8F};

void notmain() {

    parallel_setup_write();

    printk("ONE: Ready to send\n");

    for (unsigned i = 0; i < 8; i++) {
        printk("ONE: Sending byte %x\n", msg[i]);
        parallel_write_byte(msg[i]);
    }
    
    clean_reboot();
}

   