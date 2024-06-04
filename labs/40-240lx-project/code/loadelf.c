#include "rpi.h"
#include "full-except.h"
#include "fat32-arjun.h"
#include "elf32.h"
#include "syscall-handler.h"

void notmain() {
    full_except_install(0);
    full_except_set_syscall(syscall_handler);
    
    uint32_t cpsr_cur = cpsr_get();
    assert(mode_get(cpsr_cur) == SUPER_MODE);
    uint32_t cpsr_A = mode_set(cpsr_cur, USER_MODE);


    struct fat32_filesystem fs = fat32init(1);
    uint8_t buffer[SECTOR];

    struct fat32_directory_entry binary;
    // assert(traverseToFile(&fs, (const char *[]){"bin", "hello"}, 2, fs.root_cluster, &binary));
    // assert(traverseToFile(&fs, (const char *[]){"bin", "echo"}, 2, fs.root_cluster, &binary));
    assert(traverseToFile(&fs, (const char *[]){"bin", "cat"}, 2, fs.root_cluster, &binary));

    uint8_t *binary_buffer = kmalloc(((binary.size + SECTOR - 1) / SECTOR) * SECTOR);
    uint32_t binary_cluster = ENTRY_CLUSTER(binary);
    int i = 0;
    while (i < binary.size) {
        readSector(&binary_buffer[i], CLUSTER_TO_SECTOR(fs, binary_cluster) + (i / SECTOR) % fs.sectors_per_cluster);
        i += SECTOR;
        if (i % (SECTOR * fs.sectors_per_cluster) == 0)
            binary_cluster = nextCluster(&fs, binary_cluster);
    }

    // const char *argv[] = {"hello");
    // const char *argv[] = {"echo", "hello", "world!"};
    const char *argv[] = {"cat", "config.txt"};
    run_elf32(binary_buffer, sizeof(argv) / sizeof(*argv), argv);

    printk("Success!: %s\n", __FILE__);
    clean_reboot();
}