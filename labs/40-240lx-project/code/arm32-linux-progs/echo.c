#include "syscalls.h"

#define STDOUT_FD 1

int strlen(const char *str) {
    const char *s = str;
    while (*s++);
    return s - str - 1;
}

void truemain(int argc, char *argv[]) {
    for (++argv; --argc; ++argv) {
        write(STDOUT_FD, *argv, strlen(*argv));
        if (argc > 1) write(STDOUT_FD, " ", 1);
        else write(STDOUT_FD, "\n", 1);
    }
}