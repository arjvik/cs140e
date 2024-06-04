#include "syscalls.h"
#define assert(cond, msg) if (!(cond)) { write(STDOUT_FD, msg, strlen(msg)); exit(1); }

#define STDOUT_FD 1
#define BUF_SIZE 102400
#define O_RDONLY 0x0
#define O_DIRECTORY 0x200000
#define AT_FDCWD -100


void reverse(char str[], int length) {
    int start = 0; int end = length - 1; while (start < end) { char temp = str[start]; str[start] = str[end]; str[end] = temp; start++; end--; }
}
void intToStr(int num, char str[]) {
    int i = 0; int isNegative = 0;if (num == 0) { str[i++] = '0'; str[i] = '\0'; return; }
    if (num < 0) { isNegative = 1; num = -num; }
    while (num != 0) { int rem = num % 10; str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0'; num = num / 10; }
    if (isNegative) { str[i++] = '-'; } str[i] = '\0'; reverse(str, i);
}

int strlen(const char *str) {
    const char *s = str;
    while (*s++);
    return s - str - 1;
}

void truemain() {
    char buf[BUF_SIZE];

    int fd = openat(-100, ".", O_RDONLY | O_DIRECTORY, 0);

    assert(fd > 2, "open failed");
    write(STDOUT_FD, "Contents of directory:\n", 23);

    int nread = getdents(fd, buf, BUF_SIZE);
    char str[100];
    intToStr(nread, str);
    write(STDOUT_FD, "nread: ", 7);
    write(STDOUT_FD, str, strlen(str));
    write(STDOUT_FD, "\n", 1);
    assert(nread > 0, "getdents failed");

    close(fd);
    exit(0);
}