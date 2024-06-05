// implement:
//  void uart_init(void)
//
//  int uart_can_get8(void);
//  int uart_get8(void);
//
//  int uart_can_put8(void);
//  void uart_put8(uint8_t c);
//
//  int uart_tx_is_empty(void) {
//
// see that hello world works.
//
//
#include "rpi.h"

#define MASK_ON(reg, mask) PUT32((reg), GET32((reg)) | (mask))
#define MASK_OFF(reg, mask) PUT32((reg), GET32((reg)) & ~(mask))

enum {
    AUXENB = 0x20215004,
    AUX_MU_IO_REG = 0x20215040,
    AUX_MU_IER_REG = 0x20215044,
    AUX_MU_IIR_REG = 0x20215048,
    AUX_MU_LCR_REG = 0x2021504C,
    AUX_MU_LSR_REG = 0x20215054,
    AUX_MU_CNTL_REG = 0x20215060,
    AUX_MU_STAT_REG = 0x20215064,
    AUX_MU_BAUD_REG = 0x20215068,
};

#define SYSTEM_CLOCK_FREQ 250 ## 000 ## 000
#define DESIRED_BAUD_RATE 115200
#define BAUD_VALUE 271
// (((SYSTEM_CLOCK_FREQ / DESIRED_BAUD_RATE) / 8) - 1)

// called first to setup uart to 8n1 115200  baud,
// no interrupts.
//  - you will need memory barriers, use <dev_barrier()>
//
//  later: should add an init that takes a baud rate.
void uart_init(void) {
    dev_barrier();
    gpio_set_function(GPIO_TX, GPIO_FUNC_ALT5); // set TX and RX pins to UART functionality
    gpio_set_function(GPIO_RX, GPIO_FUNC_ALT5);
    gpio_set_on(GPIO_TX); // enable pull up/down
    dev_barrier();
    MASK_ON(AUXENB, 0x1); // enable mini uart
    // MASK_OFF(AUX_MU_CNTL_REG, 0x3); // disable transmitter and receiver
    PUT32(AUX_MU_CNTL_REG, 0); // disable transmitter and receiver
    // dev_barrier();
    // MASK_ON(AUX_MU_IIR_REG, 0x6); // clear rx and tx FIFOs
    PUT32(AUX_MU_IIR_REG, 0x6); // clear rx and tx FIFOs
    // MASK_OFF(AUX_MU_IER_REG, 0x3); // disable interrupts
    PUT32(AUX_MU_IER_REG, 0); // disable interrupts
    // MASK_ON(AUX_MU_LCR_REG, 0x3); // set data size to 8 bits
    PUT32(AUX_MU_LCR_REG, 0x3); // set data size to 8 bits
    PUT32(AUX_MU_BAUD_REG, BAUD_VALUE); // set baud rate
    // MASK_ON(AUX_MU_CNTL_REG, 0x3); // enable transmitter and receiver
    PUT32(AUX_MU_CNTL_REG, 0x3); // enable transmitter and receiver
    dev_barrier();
}

// disable the uart.
void uart_disable(void) {
    dev_barrier();
    uart_flush_tx();
    MASK_OFF(AUXENB, 0x1); // disable mini uart
    dev_barrier();
}


// returns one byte from the rx queue, if needed
// blocks until there is one.
int uart_get8(void) {
    dev_barrier();
    while(!uart_has_data())
        ;
    return GET32(AUX_MU_IO_REG) & 0xFF;
}

// 1 = space to put at least one byte, 0 otherwise.
int uart_can_put8(void) {
    return GET32(AUX_MU_STAT_REG) & 0x2;
}

// put one byte on the tx qqueue, if needed, blocks
// until TX has space.
// returns < 0 on error.
int uart_put8(uint8_t c) {
    while (!uart_can_put8())
        ;
    PUT32(AUX_MU_IO_REG, c & 0xFF);
    return 0;
}

// simple wrapper routines useful later.

// 1 = at least one byte on rx queue, 0 otherwise
int uart_has_data(void) {
    return GET32(AUX_MU_STAT_REG) & 0x1;
}

// return -1 if no data, otherwise the byte.
int uart_get8_async(void) { 
    if(!uart_has_data())
        return -1;
    return uart_get8();
}

// 1 = tx queue empty, 0 = not empty.
int uart_tx_is_empty(void) {
    return GET32(AUX_MU_LSR_REG) & 0x40;
}

// flush out all bytes in the uart --- we use this when 
// turning it off / on, etc.
void uart_flush_tx(void) {
    while(!uart_tx_is_empty())
        ;
}
