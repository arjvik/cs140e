#include "rpi.h"
#include "full-except.h"
#include "fat32-arjun.h"
#include "elf32.h"
#include "syscall-handler.h"
#include "longjmp.h"

char *readline(char *buffer, int size) {
    int i = 0;
    while (true) {
        assert(i < size);
        char c = uart_get8();
        if (c == '\n') {
            buffer[i] = '\0';
            return buffer;
        }
        buffer[i++] = c;
    }
}

int strcmp(const char *s1, const char *s2) {
    while (*s1 && *s2 && *s1 == *s2) {
        s1++; s2++;
    }
    return *s1 - *s2;
}

void notmain() {
    full_except_install(0);
    full_except_set_syscall(syscall_handler);
    
    uint32_t cpsr_cur = cpsr_get();
    assert(mode_get(cpsr_cur) == SUPER_MODE);
    uint32_t cpsr_A = mode_set(cpsr_cur, USER_MODE);


    struct fat32_filesystem fs = fat32init(1);
    uint8_t buffer[SECTOR];

    printk("\n\n\n");
    
    setjmp();
    bool found = false;
    struct fat32_directory_entry binary;
    char *argv[1024];
    int argc = 0;
    do {
        char command[1024];
        command[0] = 0;
        printk("$ ");
        readline(command, sizeof(command));
        while (!*command) {
            printk("$ ");
            readline(command, sizeof(command));
        }

        if (strcmp(command, "exit") == 0) {
            clean_reboot();
        }
        
        // split command by consecutive spaces
        argv[argc++] = command;
        for (char *c = command; *c; c++) {
            if (*c == ' ') {
                *c = '\0';
                while (*(c + 1) == ' ')
                    c++;
                argv[argc++] = c + 1;
            }
        }
        assert(argc > 0);
        assert(**argv);

        if (!traverseToFile(&fs, (const char *[]){"bin", argv[0]}, 2, fs.root_cluster, &binary)) {
            printk("shell: command not found: %s\n", argv[0]);
        } else {
            found = true;
        }
    } while (!found);

    uint8_t *binary_buffer = kmalloc(((binary.size + SECTOR - 1) / SECTOR) * SECTOR);
    uint32_t binary_cluster = ENTRY_CLUSTER(binary);
    int i = 0;
    while (i < binary.size) {
        readSector(&binary_buffer[i], CLUSTER_TO_SECTOR(fs, binary_cluster) + (i / SECTOR) % fs.sectors_per_cluster);
        i += SECTOR;
        if (i % (SECTOR * fs.sectors_per_cluster) == 0)
            binary_cluster = nextCluster(&fs, binary_cluster);
    }

    run_elf32(binary_buffer, argc, (const char **) argv);

    clean_reboot();
}