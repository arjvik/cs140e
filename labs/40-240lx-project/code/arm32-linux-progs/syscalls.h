#include "syscall-interface.h"

#define SYS_EXIT  1
static inline void exit(int status) { syscall1(SYS_EXIT, status); }

#define SYS_WRITE 4
static inline int write(int fd, const void *buf, int count) { return syscall3(SYS_WRITE, fd, (int) buf, count); }