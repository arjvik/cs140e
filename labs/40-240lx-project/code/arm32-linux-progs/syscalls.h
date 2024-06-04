#include "syscall-interface.h"
#include "../syscalls-common.h"

static inline void exit(int status) { syscall1(SYS_EXIT, status); }

static inline int read(int fd, void *buf, int count) { return syscall3(SYS_READ, fd, (int) buf, count); }

static inline int write(int fd, const void *buf, int count) { return syscall3(SYS_WRITE, fd, (int) buf, count); }

static inline int open(const char *pathname, int flags, int mode) { return syscall3(SYS_OPEN, (int) pathname, flags, mode); }

static inline int openat(int dirfd, const char *pathname, int flags, int mode) { return syscall4(SYS_OPENAT, dirfd, (int) pathname, flags, mode); }

static inline int close(int fd) { return syscall1(SYS_CLOSE, fd); }

static inline int getdents(int fd, void *dirp, int count) { return syscall3(SYS_GETDENTS, fd, (int) dirp, count); }