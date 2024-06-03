#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
// #include <assert.h>
#define assert(x) if (!(x)) { panic("Assertion failed: %s\n", #x); }
#include "rpi.h"
#include "fat32-arjun.h"

void notmain() {
    printk("Starting SD Driver test.\n");

    printk("Initializing...\n");
    struct fat32_filesystem fs = fat32init(1);
    uint8_t buffer[SECTOR];

    printk("Reading 'BUSYBOX   ':\n");
    struct fat32_directory_entry binary;
    assert(findFile(&fs, "BUSYBOX    ", fs.root_cluster, &binary));
    printk("binary->name: %c%c%c%c%c%c%c%c%c%c%c\n", binary.name[0], binary.name[1], binary.name[2], binary.name[3], binary.name[4], binary.name[5], binary.name[6], binary.name[7], binary.name[8], binary.name[9], binary.name[10]);
    printk("binary->size: %u KB\n", (uint32_t) binary.size / 1024);
    printk("binary->address: %x\n", CLUSTER_TO_SECTOR(fs, ENTRY_CLUSTER(binary)));
    readSector(buffer, CLUSTER_TO_SECTOR(fs, ENTRY_CLUSTER(binary)));
    printk("First 8 bytes:\n%c%c%c%c%c%c%c%c\n", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6], buffer[7]);

    // uint8_t binary_buffer[((binary.size + SECTOR - 1) / SECTOR) * SECTOR];
    uint8_t *binary_buffer = kmalloc(((binary.size + SECTOR - 1) / SECTOR) * SECTOR);
    printk("binary_buffer: %x\n", binary_buffer);
    uint32_t binary_cluster = ENTRY_CLUSTER(binary);
    int i = 0;
    while (i < binary.size) {
        readSector(&binary_buffer[i], CLUSTER_TO_SECTOR(fs, binary_cluster) + (i / SECTOR) % fs.sectors_per_cluster);
        i += SECTOR;
        if (i % (SECTOR * fs.sectors_per_cluster) == 0)
            binary_cluster = nextCluster(&fs, binary_cluster);
    }

    printk("Success!: %s\n", __FILE__);
    clean_reboot();
}