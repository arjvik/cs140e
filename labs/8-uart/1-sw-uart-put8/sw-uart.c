#include <stdint.h>
#include "rpi.h"
#include "sw-uart.h"
#include "cycle-count.h"
#include "cycle-util.h"
#include "wait-routines.h"

#include <stdarg.h>

// do this first: used timed_write to cleanup.
//  recall: time to write each bit (0 or 1) is in <uart->usec_per_bit>
void sw_uart_put8(sw_uart_t *uart, uint8_t c) {
    // use local variables to minimize any loads or stores
    int tx = uart->tx;
    uint32_t n = uart->cycle_per_bit,
             u = n,
             s = cycle_cnt_read();
    
    // start bit
#define write_and_wait(v) do { gpio_write(tx, (v)); wait_ncycles_exact(s, u); u += n; } while (0);
    write_and_wait(0);
    write_and_wait((c >> 0) & 1);
    write_and_wait((c >> 1) & 1);
    write_and_wait((c >> 2) & 1);
    write_and_wait((c >> 3) & 1);
    write_and_wait((c >> 4) & 1);
    write_and_wait((c >> 5) & 1);
    write_and_wait((c >> 6) & 1);
    write_and_wait((c >> 7) & 1);
    write_and_wait(1);
}

// do this second: you can type in pi-cat to send stuff.
//      EASY BUG: if you are reading input, but you do not get here in 
//      time it will disappear.
int sw_uart_get8_timeout(sw_uart_t *uart, uint32_t timeout_usec) {
    unsigned rx = uart->rx;

    // right away (used to be return never).
    while(!wait_until_usec(rx, 0, timeout_usec))
        return -1;

    uint32_t n = uart->cycle_per_bit,
             u = n + (n >> 1),
             s = cycle_cnt_read();
    uint8_t c = 0;
#define wait_and_read() ({ wait_ncycles_exact(s, u); u += n; (gpio_read(rx)); })
    c |= wait_and_read() << 0;
    c |= wait_and_read() << 1;
    c |= wait_and_read() << 2;
    c |= wait_and_read() << 3;
    c |= wait_and_read() << 4;
    c |= wait_and_read() << 5;
    c |= wait_and_read() << 6;
    c |= wait_and_read() << 7;
    dev_barrier();
    assert(wait_and_read() == 1);
    return c;
}

// finish implementing this routine.  
sw_uart_t sw_uart_init_helper(unsigned tx, unsigned rx,
        unsigned baud, 
        unsigned cyc_per_bit,
        unsigned usec_per_bit) 
{
    assert(tx && tx<31);
    assert(rx && rx<31);
    assert(cyc_per_bit && cyc_per_bit > usec_per_bit);
    assert(usec_per_bit);

    // make sure it makes sense.
    unsigned mhz = 700 * 1000 * 1000;
    unsigned derived = cyc_per_bit * baud;
    if(! ((mhz - baud) <= derived && derived <= (mhz + baud)) )
        panic("too much diff: cyc_per_bit = %d * baud = %d\n", 
            cyc_per_bit, cyc_per_bit * baud);

    gpio_set_input(rx);
    gpio_set_output(tx);
    gpio_write(tx, 1);

    return (sw_uart_t) { 
            .tx = tx, 
            .rx = rx, 
            .baud = baud, 
            .cycle_per_bit = cyc_per_bit ,
            .usec_per_bit = usec_per_bit 
    };
}
