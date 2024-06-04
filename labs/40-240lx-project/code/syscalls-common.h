#define SYS_EXIT  1
#define SYS_READ 3
#define SYS_WRITE 4
#define SYS_OPEN    5
#define SYS_CLOSE   6
#define SYS_GETDENTS 141
#define SYS_OPENAT 322

struct linux_dirent {
    int inode;
    int offset;
    unsigned short d_reclen;
    char d_name[];
};