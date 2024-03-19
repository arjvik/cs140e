#include <stdint.h>
#include "rpi.h"
#include "parallel.h"

uint8_t msg[] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};

void notmain() {

    parallel_setup_read();

    printk("TWO: Ready to receive.\n");

    for (unsigned i = 0; i < 8; i++) {
        uint8_t byte = parallel_read_byte();
        printk("TWO: Read byte %x\n", byte);
    }

    parallel_setup_write();
    
    printk("TWO: Ready to send.\n");

    for (unsigned i = 0; i < 8; i++) {
        printk("TWO: Sending byte %x\n", msg[i]);
        parallel_write_byte(msg[i]);
    }

    parallel_setup_read();

    printk("TWO: Ready to receive.\n");
    for (unsigned i = 0; i < 2; i++) {
        uint8_t byte = parallel_read_byte();
        printk("TWO: Read byte %x\n", byte);
    }

    parallel_setup_write();

    printk("TWO: Ready to send.\n");
    parallel_write_byte(0x00);
    parallel_write_byte(0xFF);

    clean_reboot();
}