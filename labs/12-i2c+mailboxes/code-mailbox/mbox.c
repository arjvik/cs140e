#include "rpi.h"
#include "mbox.h"

// dump out the entire messaage.  useful for debug.
void msg_dump(const char *msg, volatile uint32_t *u, unsigned nwords) {
    printk("%s\n", msg);
    for(int i = 0; i < nwords; i++)
        output("u[%d]=%x\n", i,u[i]);
}

/*
  This is given.

  Get board serial
    Tag: 0x00010004
    Request: Length: 0
    Response: Length: 8
    Value: u64: board serial
*/
uint64_t rpi_get_serialnum(void) {
    // 16-byte aligned 32-bit array
    volatile uint32_t msg[8] __attribute__((aligned(16))) = {
        sizeof(msg),       // total size in bytes.
        0,         // sender: always 0.
        0x00010004,  // serial tag
        8,           // total bytes avail for reply
        0,           // request code [0].
        0,           // space for 1st word of reply 
        0,           // space for 2nd word of reply 
        0   // end tag
    };

    // make sure aligned
    assert((unsigned)msg%16 == 0);

    // send and receive message
    mbox_send(MBOX_CH, msg);

    // output("got:\n");
    // for(int i = 0; i < sizeof(msg) / sizeof(msg[0]); i++)
    //     output("msg[%d]=%x\n", i, msg[i]);

    // should have value for success: 1<<31
    if(msg[1] != 0x80000000)
		panic("invalid response: got %x\n", msg[1]);

    // high bit should be set and reply size
    assert(msg[4] == ((1<<31) | 8));

    // for me the upper 32 bits were never non-zero.  
    // not sure if always true?
    assert(msg[6] == 0);
    
    return msg[5];
}

uint32_t rpi_get_memsize(void) {
    // 16-byte aligned 32-bit array
    volatile uint32_t msg[8] __attribute__((aligned(16))) = {
        sizeof(msg),       // total size in bytes.
        0,         // sender: always 0.
        0x00010005,  // serial tag
        8,           // total bytes avail for reply
        0,           // request code [0].
        0,           // space for 1st word of reply 
        0,           // space for 2nd word of reply 
        0   // end tag
    };

    // make sure aligned
    assert((unsigned)msg%16 == 0);

    // send and receive message
    mbox_send(MBOX_CH, msg);

    // output("got:\n");
    // for(int i = 0; i < sizeof(msg) / sizeof(msg[0]); i++)
    //     output("msg[%d]=%x\n", i, msg[i]);

    // should have value for success: 1<<31
    if(msg[1] != 0x80000000)
		panic("invalid response: got %x\n", msg[1]);

    // high bit should be set and reply size
    assert(msg[4] == ((1<<31) | 8));

    return msg[6];
}

uint32_t rpi_get_model(void) {
    // 16-byte aligned 32-bit array
    volatile uint32_t msg[7] __attribute__((aligned(16))) = {
        sizeof(msg),       // total size in bytes.
        0,         // sender: always 0.
        0x00010001,  // serial tag
        4,           // total bytes avail for reply
        0,           // request code [0].
        0,           // space for 1st word of reply 
        0   // end tag
    };

    // make sure aligned
    assert((unsigned)msg%16 == 0);

    // send and receive message
    mbox_send(MBOX_CH, msg);

    // output("got:\n");
    // for(int i = 0; i < sizeof(msg) / sizeof(msg[0]); i++)
    //     output("msg[%d]=%x\n", i, msg[i]);

    // should have value for success: 1<<31
    if(msg[1] != 1<<31)
		panic("invalid response: got %x\n", msg[1]);

    // high bit should be set and reply size
    assert(msg[4] == ((1<<31) | 4));

    return msg[5];
}

// https://www.raspberrypi-spy.co.uk/2012/09/checking-your-raspberry-pi-board-version/
uint32_t rpi_get_revision(void) {
    // 16-byte aligned 32-bit array
    volatile uint32_t msg[7] __attribute__((aligned(16))) = {
        sizeof(msg),       // total size in bytes.
        0,         // sender: always 0.
        0x00010002,  // serial tag
        4,           // total bytes avail for reply
        0,           // request code [0].
        0,           // space for 1st word of reply 
        0   // end tag
    };

    // make sure aligned
    assert((unsigned)msg%16 == 0);

    // send and receive message
    mbox_send(MBOX_CH, msg);

    // output("got:\n");
    // for(int i = 0; i < sizeof(msg) / sizeof(msg[0]); i++)
    //     output("msg[%d]=%x\n", i, msg[i]);

    // should have value for success: 1<<31
    if(msg[1] != 1<<31)
		panic("invalid response: got %x\n", msg[1]);

    // high bit should be set and reply size
    assert(msg[4] == ((1<<31) | 4));

    return msg[5];
}

uint32_t rpi_temp_get(void) {
    // 16-byte aligned 32-bit array
    volatile uint32_t msg[8] __attribute__((aligned(16))) = {
        sizeof(msg),       // total size in bytes.
        0,         // sender: always 0.
        0x00030006,  // serial tag
        8,           // total bytes avail for reply
        0,           // request code [0].
        0,           // space for 1st word of reply
        0,           // space for 2nd word of reply 
        0   // end tag
    };

    // make sure aligned
    assert((unsigned)msg%16 == 0);

    // send and receive message
    mbox_send(MBOX_CH, msg);

    // output("got:\n");
    // for(int i = 0; i < sizeof(msg) / sizeof(msg[0]); i++)
    //     output("msg[%d]=%x\n", i, msg[i]);

    // should have value for success: 1<<31
    if(msg[1] != 1<<31)
		panic("invalid response: got %x\n", msg[1]);

    // high bit should be set and reply size
    assert(msg[4] == ((1<<31) | 8));

    return msg[6];
}
