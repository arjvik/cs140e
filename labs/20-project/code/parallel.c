#include <stdbool.h>
#include "rpi.h"
#include "parallel.h"

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

void parallel_setup_read(void) {
    for (unsigned pin = par_pins_start; pin < par_pins_start + 8; pin++) {
        gpio_set_pullup(pin);
        gpio_set_input(pin);
    }
    gpio_set_off(par_listening_pin);
    gpio_set_output(par_listening_pin);
    gpio_set_pulldown(par_writing_pin);
    gpio_set_input(par_writing_pin);
}

uint8_t parallel_read_byte(void) {
    static bool last = false;
    gpio_write(par_listening_pin, !last);
    while (gpio_read(par_writing_pin) == last)
        ;
    last = !last;
    uint32_t value = (get32((uint32_t *) gpio_lev0 + (par_pins_start/32)) >> par_pins_start) & 0xFF;
    return value;
}

void parallel_setup_write(void) {
    for (unsigned pin = par_pins_start; pin < par_pins_start + 8; pin++) {
        gpio_set_off(pin);
        gpio_set_output(pin);
    }
    gpio_set_pulldown(par_listening_pin);
    gpio_set_input(par_listening_pin);
    gpio_set_off(par_writing_pin);
    gpio_set_output(par_writing_pin);
}

void parallel_write_byte(uint8_t byte) {
    static bool last = false;
    while (gpio_read(par_listening_pin) == last)
        ;
    uint32_t set = (uint32_t) byte, clr = ((uint32_t) (~byte)) << par_pins_start;
    put32((uint32_t *) gpio_set0 + (par_pins_start/32), set);
    put32((uint32_t *) gpio_clr0 + (par_pins_start/32), clr);
    last = !last;
    gpio_write(par_writing_pin, last);
}