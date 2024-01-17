// part 1: uses your GPIO code to blink a single LED connected to 
// pin 20.
//   - when run should blink 10 times.  
//   - you will have to restart the pi by pulling the usb connection out.
#include "rpi.h"

void notmain(void) {
    for (int j = 0; j < 20; j++)
        gpio_set_output(j);
    while(1) {
        for (int j = 0; j < 20; j++)
            gpio_set_on(j);
        delay_cycles(1000000);
        for (int j = 0; j < 20; j++)
            gpio_set_off(j);
        delay_cycles(1000000);
    }
}
