#include "rpi.h"

enum {
    par_pins_start = 0,
    par_listening_pin = 20,
    par_writing_pin = 21,
};


uint8_t gpio_read_byte() {
    enum {
        GPIO_BASE = 0x20200000,
        gpio_set0  = (GPIO_BASE + 0x1C),
        gpio_clr0  = (GPIO_BASE + 0x28),
        gpio_lev0  = (GPIO_BASE + 0x34)
    };
    uint32_t value = get32((uint32_t *) gpio_lev0 + (par_pins_start/32));
    return (value >> par_pins_start) & 0xFF;
}

void notmain() {

    for (unsigned pin = par_pins_start; pin < par_pins_start + 8; pin++) {
        gpio_set_pullup(pin);
        gpio_set_input(pin);
    }
    gpio_set_off(par_listening_pin);
    gpio_set_output(par_listening_pin);
    gpio_set_pulldown(par_writing_pin);
    gpio_set_input(par_writing_pin);


    printk("SERVER: Ready to receive.\n");

    int last = 0;

    for (unsigned i = 0; i < 8; i++) {
        last = !last;
        gpio_write(par_listening_pin, last);
        while (gpio_read(par_writing_pin) != last)
            ;
        uint8_t byte = gpio_read_byte();
        printk("SERVER: Read byte %x\n", byte);
    }
    
    clean_reboot();
}