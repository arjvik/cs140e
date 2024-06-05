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

void truemain(int argc, char *argv[]) {
    char buf[BUF_SIZE];

    if (argc <= 1) {
        write(STDOUT_FD, "Usage: cat <file> [<file> [<file> [...]]]\n", 42);
        exit(1);
    }
    for (int i = 1; i < argc; i++) {
        int fd = open(argv[i], O_RDONLY, 0);
        assert(fd > 2, "open failed");

        int nread;
        while ((nread = read(fd, buf, BUF_SIZE)) > 0)
            write(STDOUT_FD, buf, nread);
        assert(!(nread < 0), "read failed");
    }
}