#include "syscalls.h"
#define assert(cond, msg) if (!(cond)) { write(STDOUT_FD, msg, strlen(msg)); exit(1); }

#define STDOUT_FD 1
#define BUF_SIZE 10240
#define O_RDONLY 0x0

int strlen(const char *str) {
    const char *s = str;
    while (*s++);
    return s - str - 1;
}

// Originally for debugging, but clearly I need this. GPT generated, no correctness promised :)
void reverse(char str[], int length) {
    int start = 0; int end = length - 1; while (start < end) { char temp = str[start]; str[start] = str[end]; str[end] = temp; start++; end--; }
}
void intToStr(int num, char str[]) {
    int i = 0; int isNegative = 0;if (num == 0) { str[i++] = '0'; str[i] = '\0'; return; }
    if (num < 0) { isNegative = 1; num = -num; }
    while (num != 0) { int rem = num % 10; str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0'; num = num / 10; }
    if (isNegative) { str[i++] = '-'; } str[i] = '\0'; reverse(str, i);
}

// #define CRC32_POLY 0x04C11DB7
#define CRC32_INIT 0xFFFFFFFF
#define CRC32_POLY 0x04C11DB7

unsigned int crc32(unsigned int crc, const void *data, unsigned int length) {
    const unsigned char *bytes = (const unsigned char *)data;

    crc ^= CRC32_INIT;

    while (length--) {
        crc ^= *bytes++ << 24;
        for (int i = 0; i < 8; i++) {
            if (crc & 0x80000000)
                crc = (crc << 1) ^ CRC32_POLY;
            else
                crc <<= 1;
        }
    }

    return crc ^ ~CRC32_INIT;
}

void truemain(int argc, char *argv[]) {

    if (argc <= 1) {
        write(STDOUT_FD, "Usage: cksum <file> [<file> [<file> [...]]]\n", 44);
        exit(1);
    }
    for (int i = 1; i < argc; i++) {
        int fd = open(argv[i], O_RDONLY, 0);
        assert(fd > 2, "open failed");

        char buf[BUF_SIZE];
        int nread;
        unsigned int crc = 0;
        unsigned int total_bytes = 0;
        while ((nread = read(fd, buf, BUF_SIZE)) > 0) {
            crc = crc32(crc, buf, nread);
            total_bytes += nread;
        }
        assert(!(nread < 0), "read failed");

        intToStr(crc, buf);
        write(STDOUT_FD, buf, strlen(buf));
        write(STDOUT_FD, " ", 1);
        intToStr(total_bytes, buf);
        write(STDOUT_FD, buf, strlen(buf));
        write(STDOUT_FD, " ", 1);
        write(STDOUT_FD, argv[i], strlen(argv[i]));
        write(STDOUT_FD, "\n", 1);
    }
}