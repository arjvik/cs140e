/*
 * Implement the following routines to set GPIO pins to input or output,
 * and to read (input) and write (output) them.
 *
 * DO NOT USE loads and stores directly: only use GET32 and PUT32
 * to read and write memory.  Use the minimal number of such calls.
 *
 * See rpi.h in this directory for the definitions.
 */
#include "rpi.h"

// see broadcomm documents for magic addresses.
enum {
    GPIO_BASE = 0x20200000,
    gpio_set0  = (GPIO_BASE + 0x1C),
    gpio_clr0  = (GPIO_BASE + 0x28),
    gpio_lev0  = (GPIO_BASE + 0x34)
};

//
// Part 1 implement gpio_set_on, gpio_set_off, gpio_set_output
//

// set <pin> to be an output pin.
//
// note: fsel0, fsel1, fsel2 are contiguous in memory, so you
// can (and should) use array calculations!
void gpio_set_output(unsigned pin) {
    if(pin >= 32)
        return;
    unsigned value = get32((unsigned *) GPIO_BASE + (pin / 10));
    value &= ~(0x7 << ((pin % 10) * 3));
    value |= 0x1 << ((pin % 10) * 3);
    put32(((unsigned *) GPIO_BASE + (pin / 10)), value);
}

// set GPIO <pin> on.
void gpio_set_on(unsigned pin) {
    if(pin >= 32)
        return;
    put32((unsigned *) gpio_set0 + (pin / 32), 1 << (pin % 32));
}

// set GPIO <pin> off
void gpio_set_off(unsigned pin) {
    if(pin >= 32)
        return;
   put32((unsigned *) gpio_clr0 + (pin / 32), 1 << (pin % 32));
}

// set <pin> to <v> (v \in {0,1})
void gpio_write(unsigned pin, unsigned v) {
    if(v)
        gpio_set_on(pin);
    else
        gpio_set_off(pin);
}

//
// Part 2: implement gpio_set_input and gpio_read
//

// set <pin> to input.
void gpio_set_input(unsigned pin) {
    if(pin >= 32)
        return;
    unsigned value = get32((unsigned *) GPIO_BASE + (pin / 10));
    value &= ~(0x7 << ((pin % 10) * 3));
    put32((unsigned *) GPIO_BASE + (pin / 10), value);
}

// return the value of <pin>
int gpio_read(unsigned pin) {
    if(pin >= 32)
        return -1;
    return (get32((unsigned *) gpio_lev0 + (pin / 32)) & (1 << (pin % 32))) >> (pin % 32);
}