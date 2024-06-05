#include <stdio.h>
#include "rpi.h"
#include "syscalls-common.h"
#include "syscall-handler.h"
#include "fat32-arjun.h"
#include "longjmp.h"

#define PATH_MAX 256
#define SEGMENT_MAX 12
#define FD_MAX 10

struct {
    bool opened;
    struct fat32_directory_entry file;
    size_t offset;
} fd_table[FD_MAX];
int next_fd = 3;

struct fat32_filesystem fs;
bool fs_inited = false;

int sys_exit(int status) {
    for (int i = 3; i < FD_MAX; i++)
        fd_table[i].opened = false;
    next_fd = 3;
    if (jmp_set) {
        longjmp(status);
    } else {
        if (status != 0)
            printk("Exiting with nonzero status %d! (forcing clean exit anyway)\n", status);
        clean_reboot();
    }
}

int sys_open(const char *pathname, int flags, int mode) {
    if (!fs_inited) {
        fs = fat32init(1);
        fs_inited = true;
    }
    
    assert(mode == 0);
    assert(next_fd < FD_MAX);
    assert(strlen(pathname) < PATH_MAX);
    
    int fd = next_fd++;

    // split path by /
    if (pathname[0] == '/') pathname++;
    char pathbuffer[PATH_MAX];
    strcpy(pathbuffer, pathname);
    char *path[SEGMENT_MAX];
    unsigned segments = 0;
    path[segments++] = pathbuffer;
    for (char *c = pathbuffer; *c; c++) {
        if (*c == '/') {
            *c = '\0';
            path[segments++] = c + 1;
        }
    }

    if (!traverseToFile(&fs, (const char **) path, segments, fs.root_cluster, &fd_table[fd].file)) {
        printk("open: file not found\n");
        sys_exit(1);
    }

    fd_table[fd].offset = 0;
    fd_table[fd].opened = true;
    return fd;
}

int sys_read(int fd, void *buf, size_t count) {
    assert(fd > 2 && fd < FD_MAX);
    assert(fd_table[fd].opened);
    assert(fs_inited);

    count = count < fd_table[fd].file.size - fd_table[fd].offset ? count : fd_table[fd].file.size - fd_table[fd].offset;
    int i = 0;
    uint32_t file_cluster = ENTRY_CLUSTER(fd_table[fd].file);
    while (i < fd_table[fd].offset + count) {
        if (i + SECTOR >= fd_table[fd].offset) {
            uint8_t buffer[SECTOR];
            readSector(buffer, CLUSTER_TO_SECTOR(fs, file_cluster) + (i / SECTOR) % fs.sectors_per_cluster);
            unsigned start_bytes = i < fd_table[fd].offset ? fd_table[fd].offset - i : 0;
            unsigned end_bytes = i + SECTOR > fd_table[fd].offset + count ? fd_table[fd].offset + count - i : SECTOR;
            memcpy(&((char *)buf)[i + start_bytes - fd_table[fd].offset], &buffer[start_bytes], end_bytes - start_bytes);
        }
        i += SECTOR;
        if (i % (SECTOR * fs.sectors_per_cluster) == 0)
            file_cluster = nextCluster(&fs, file_cluster);
    }
    fd_table[fd].offset += count;
    return count;
    // char file_buffer[((file.size + SECTOR - 1) / SECTOR) * SECTOR];
    // uint32_t file_cluster = ENTRY_CLUSTER(file);
    // int i = 0;
    // while (i < file.size) {
    //     readSector(&file_buffer[i], CLUSTER_TO_SECTOR(fs, file_cluster) + (i / SECTOR) % fs.sectors_per_cluster);
    //     i += SECTOR;
    //     if (i % (SECTOR * fs.sectors_per_cluster) == 0)
    //         file_cluster = nextCluster(&fs, file_cluster);
    // }
    // memcpy(buf, &file_buffer[fd_table[fd].offset], count);
    // fd_table[fd].offset += count;
    // return count;
}

int sys_write(int fd, const void *buf, size_t count) {
    if (fd != 1 && fd != 2)
        panic("Read-only filesystem!\n");
    // for (size_t i = 0; i < count; i++)
        // uart_put8(((char *)buf)[i]);
    printk("%s", buf);
    return 1;
}

int sys_close(int fd) {
    assert(fd > 2 && fd < FD_MAX);
    assert(fd_table[fd].opened);
    fd_table[fd].opened = false;
    if (fd == next_fd - 1)
        next_fd--;
    return 0;
}
    
int handled_syscalls[] = {SYS_EXIT, SYS_READ, SYS_WRITE, SYS_OPEN, SYS_CLOSE};
int (*sys_handlers[])() = {
    [SYS_EXIT] = sys_exit,
    [SYS_READ] = sys_read,
    [SYS_WRITE] = sys_write,
    [SYS_OPEN] = sys_open,
    [SYS_CLOSE] = sys_close,
};

int syscall_handler(regs_t *r) {
    unsigned *pc = (unsigned *)(r->regs[REGS_PC] - 4);
    unsigned immediate = *pc & ((1<<24)-1);
    assert(immediate == 0);
    // if (immediate != 0)
    //     panic("instruction is %x at %x\n", *pc, pc);
    unsigned sysno = r->regs[7];

    for (int i = 0; i < sizeof(handled_syscalls) / sizeof(handled_syscalls[0]) && sysno != handled_syscalls[i]; i++)
        if (i == sizeof(handled_syscalls) / sizeof(handled_syscalls[0]) - 1)
            panic("Unknown syscall %d\n", sysno);

    // printk("        (handling syscall %d, r0=%x r1=%x r2=%x)\n", sysno, r->regs[0], r->regs[1], r->regs[2]);

    if (!sys_handlers[sysno])
        panic("Unhandled syscall %d\n", sysno);
    
    return sys_handlers[sysno](r->regs[0], r->regs[1], r->regs[2], r->regs[3]);
}