#include <stdint.h>
#include "rpi.h"
#include "parallel.h"

#define ONE_MB (1024*1024)

void fill_page(uint32_t *addr) {
    for (uint32_t i = 0; i < ONE_MB / sizeof(uint32_t); i++)
        addr[i] = i;
}

void notmain() {

    uint32_t *page  = (uint32_t *) 0x9000000,
             *page2 = (uint32_t *) 0x9100000,
             *page3 = (uint32_t *) 0xA000000;

    fill_page(page);
    fill_page(page2);
    fill_page(page3);

    printk("Ready.\n");

    parallel_setup_read();

    uint8_t *addr = (uint8_t *) parallel_read_32();

    parallel_setup_write();
    parallel_write_n(page, ONE_MB);


    clean_reboot();
}

   