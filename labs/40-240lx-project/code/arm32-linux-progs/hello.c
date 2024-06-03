#include "syscalls.h"

#define STDOUT_FD 1

void truemain() {
    const char message[] = "Hello, World!\n";
    int length = sizeof(message) - 1;

    write(STDOUT_FD, message, length);

    exit(0);
}