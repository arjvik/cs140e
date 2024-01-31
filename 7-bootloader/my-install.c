#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <termios.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/stat.h>

#include "boot-protocol.h"

#define panic(msg...) do { fprintf(stderr, "%s:%d (%s): ", __FILE__, __LINE__, __FUNCTION__); exit(1); } while(0)

typedef int fd_t;

const int timeout = 10;
const char *const endmsg = "DONE!!!";

fd_t getTTY(const char *device, speed_t baud) {
    fd_t fd = open(device, O_RDWR | O_NOCTTY | O_SYNC);
    assert(fd >= 0 && "Error opening device");
    
    struct termios tty;
    memset(&tty, 0, sizeof tty);
    cfsetspeed(&tty, baud);

    // https://github.com/rlcheng/raspberry_pi_workshop

    // disable IGNBRK for mismatched speed tests; otherwise receive break
    // as \000 chars

    // XXX: wait, does this disable break or ignore-ignore break??
    tty.c_iflag &= ~IGNBRK;         // disable break processing
    tty.c_lflag = 0;                // no signaling chars, no echo,
    // no canonical processing
    tty.c_oflag = 0;                // no remapping, no delays
    tty.c_cc[VMIN]  = 0;            // read doesn't block
    assert(timeout < 100 && timeout > 0);
    // VTIME is in .1 seconds, so have to multiply by 10.
    tty.c_cc[VTIME] = (int)(timeout * 10);             // this seems to cause issues?

	/*
	 * Setup TTY for 8n1 mode, used by the pi UART.
	 */

    // Disables the Parity Enable bit(PARENB),So No Parity 
    tty.c_cflag &= ~PARENB; 	
    // CSTOPB = 2 Stop bits,here it is cleared so 1 Stop bit 
    tty.c_cflag &= ~CSTOPB;   	
    // Clears the mask for setting the data size     
    tty.c_cflag &= ~CSIZE;	 	
    // Set the data bits = 8
    tty.c_cflag |=  CS8; 		
    // No Hardware flow Control 
    tty.c_cflag &= ~CRTSCTS;
    // Enable receiver,Ignore Modem Control lines 
    tty.c_cflag |= CREAD | CLOCAL; 	
    	
    // Disable XON/XOFF flow control both i/p and o/p
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);    
    // Non Cannonical mode 
    tty.c_iflag &= ~(ICANON | ECHO | ECHOE | ISIG);  
    // No Output Processing
    tty.c_oflag &= ~OPOST;	

    if(tcsetattr (fd, TCSANOW, &tty) != 0)
        panic("tcsetattr failed\n");
    return fd;
}

uint8_t get8(fd_t fd) {
    uint8_t b;
    assert(read(fd, &b, 1) == 1);
    return b;
}

uint32_t get32(fd_t fd) {
    uint32_t b = 0;
    b |= get8(fd);
    b |= get8(fd) << 8;
    b |= get8(fd) << 16;
    b |= get8(fd) << 24;
    return b;
}

void put8(fd_t fd, uint8_t b) {
    assert(write(fd, &b, 1) == 1);
}

void put32(fd_t fd, uint32_t b) {
    assert(write(fd, &b, 4) == 4);
}

void skip_handle_print_op(fd_t tty, uint32_t *op) {
    while (*op == PRINT_STRING) {
        uint32_t len = get32(tty);
        assert(len < 512 && "String suspiciously long");
        fprintf(stderr, "BOOT: ", len);
        for (fd_t i = 0; i < len; i++) {
            fprintf(stderr, "%c", get8(tty));
        }
        fprintf(stderr, "\n");
        *op = get32(tty); // Already aligned
    }
}

uint32_t get_op(fd_t tty) {
    uint32_t op = get32(tty);
    skip_handle_print_op(tty, &op);
    return op;
}

void *read_code(const char *name, unsigned *size) {
    FILE *f = fopen(name, "rb");
    if (!f) panic("failed to open file %s", name);
    struct stat st;
    fstat(fileno(f), &st);
    *size = st.st_size;
    uint8_t *const buf = calloc((*size + 3) & ~3, sizeof(buf[0]));
    assert(buf);
    fread(buf, sizeof(*buf), *size, f);
    fclose(f);
    return buf;
}

void boot(const char *filename, const char *device, speed_t baud) {
    uint32_t bufsize;
    char *buf = read_code(filename, &bufsize);
    uint32_t crc = crc32(buf, bufsize);
    fprintf(stderr, "CRC: %x\n", crc);

    fd_t tty = getTTY(device, baud);
    uint8_t b;
    uint32_t op = 0;
    while (op != GET_PROG_INFO) {
        // Can't use get_op because not dword aligned yet
        op = op >> 8 | (uint32_t) get8(tty) << 24;
        skip_handle_print_op(tty, &op);
    }
    put32(tty, PUT_PROG_INFO);
    put32(tty, ARMBASE);
    put32(tty, bufsize);
    put32(tty, crc);
    while ((op = get_op(tty)) == GET_PROG_INFO);
    
    if (op != GET_CODE) panic("expected GET_CODE");
    if (get32(tty) != crc) panic("Received CRC does not match sent CRC");
    put32(tty, PUT_CODE);
    for (size_t i = 0; i < bufsize; i++) {
        put8(tty, buf[i]);
    }
    if ((op = get_op(tty)) != BOOT_SUCCESS) {
        panic("Unexpected response instead of BOOT_SUCCESS: %s", boot_op_to_str(op));
        exit(1);
    }
    fprintf(stderr, ">>>>>>>>>>>>>\n");
    const char *msgptr = endmsg;
    while (*msgptr) {
        char c = get8(tty);
        if (c == *msgptr)
            msgptr++;
        else
            msgptr = endmsg;
        fprintf(stderr, "%c", c);
    }
}

fd_t main(fd_t argc, char *argv[]) {
    boot("../../cs140e-24win/firmware/hello.bin", "/dev/ttyUSB0", B115200);
}
