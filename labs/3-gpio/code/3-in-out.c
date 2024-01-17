#include "rpi.h"

void notmain(void) {
    gpio_set_output(8);
    gpio_set_input(10);
    while(1)
        gpio_write(8, gpio_read(10));
}
