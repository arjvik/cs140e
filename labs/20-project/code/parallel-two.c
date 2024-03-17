#include <stdint.h>
#include "rpi.h"
#include "parallel.h"

void notmain() {

    parallel_setup_read();

    printk("TWO: Ready to receive.\n");

    for (unsigned i = 0; i < 8; i++) {
        uint8_t byte = parallel_read_byte();
        printk("TWO: Read byte %x\n", byte);
    }
    
    clean_reboot();
}