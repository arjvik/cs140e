#include "stdbool.h"
#include "rpi.h"

enum {
    par_pins_start = 0,
    par_listening_pin = 20,
    par_writing_pin = 21,
};

enum {
    GPIO_BASE = 0x20200000,
    gpio_set0  = (GPIO_BASE + 0x1C),
    gpio_clr0  = (GPIO_BASE + 0x28),
    gpio_lev0  = (GPIO_BASE + 0x34)
};

void gpio_write_byte(uint8_t byte) {
    uint32_t set = (uint32_t) byte, clr = ((uint32_t) (~byte)) << par_pins_start;
    put32((uint32_t *) gpio_set0 + (par_pins_start/32), set);
    put32((uint32_t *) gpio_clr0 + (par_pins_start/32), clr);
}

void notmain() {

    for (unsigned pin = par_pins_start; pin < par_pins_start + 8; pin++) {
        gpio_set_off(pin);
        gpio_set_output(pin);
    }
    gpio_set_pulldown(par_listening_pin);
    gpio_set_input(par_listening_pin);
    gpio_set_off(par_writing_pin);
    gpio_set_output(par_writing_pin);

    uint8_t msg[] = {0x1D, 0x2E, 0x3A, 0x4D, 0x5B, 0x6E, 0x7E, 0x8F};
    

    int last = 0;
    printk("CLIENT: Ready to send.\n");
    
    for (unsigned i = 0; i < 8; i++) {

        while (gpio_read(par_listening_pin) == last)
            ;
        last = !last;
        printk("CLIENT: Sending byte %x\n", msg[i]);
        gpio_write_byte(msg[i]);
        gpio_write(par_writing_pin, last);
    }

    clean_reboot();
}