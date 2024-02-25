#include <stdio.h>
#include <stdint.h>
// #include <assert.h>
#define assert(x) if (!(x)) { panic("Assertion failed: %s\n", #x); }
#include "rpi.h"
#include "emmc.h"

#define SECTOR 512

struct mbr_partition_entry {
    uint8_t status;
    uint8_t chs_first[3];
    uint8_t type;
    uint8_t chs_last[3];
    uint32_t lba_first;
    uint32_t sectors;
} __attribute__((packed)); 

struct fat32_header {
    uint8_t jmp[3];
    uint8_t oem[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t fat_count;
    uint8_t unknown[19];
    uint32_t sectors_per_fat;
    uint32_t unknown2;
    uint32_t root_cluster;
    
} __attribute__((packed));

struct fat32_directory_entry {
    uint8_t name[11];
    struct {
        uint8_t read_only: 1;
        uint8_t hidden: 1;
        uint8_t system: 1;
        uint8_t volume_id: 1;
        uint8_t directory: 1;
        uint8_t archive: 1;
        uint8_t reserved: 2;
    } attributes;
    uint8_t unknown[8];
    uint16_t first_cluster_high;
    uint8_t unknown2[4];
    uint16_t first_cluster_low;
    uint32_t size;
} __attribute__((packed));
#define ENTRY_CLUSTER(e) ((((uint32_t) (e).first_cluster_high) << 16) | ((uint32_t) (e).first_cluster_low))

uint32_t sectors_per_cluster;
uint32_t lba_first;
uint32_t cluster_begin;
uint32_t fat_begin;
#define CLUSTER_TO_SECTOR(c) (cluster_begin + ((c) - 2) * sectors_per_cluster)

void readSector(uint8_t *buffer, uint32_t sector) {
    emmc_read(sector, buffer, 512);
}

uint32_t nextCluster(int c) {
    uint8_t buffer[SECTOR];
#define FAT_ENTRIES_PER_SECTOR (SECTOR / sizeof(uint32_t))
    readSector(buffer, fat_begin + c / FAT_ENTRIES_PER_SECTOR);
    return ((uint32_t *) buffer)[c % FAT_ENTRIES_PER_SECTOR];
}

struct fat32_directory_entry findFile(const char *name, uint32_t firstCluster) {
    uint8_t buffer[SECTOR];
    readSector(buffer, CLUSTER_TO_SECTOR(firstCluster));
    struct fat32_directory_entry *dir = (typeof(dir)) buffer;
    for (int i = 0; i < SECTOR / sizeof(*dir); i++) {
        assert (dir[i].name[0] != 0x00);
        // if (dir[i].name[0] == 0x00) return NULL;
        if (dir[i].name[0] == 0xE5) continue;
        if (dir[i].attributes.read_only && dir[i].attributes.hidden && dir[i].attributes.system && dir[i].attributes.volume_id) continue;
        if (dir[i].attributes.directory) continue;
        int j;
        for (j = 0; j < 11; j++)
            if (dir[i].name[j] != name[j])
                break;
        if (j == 11)
            return dir[i];
    }
    return findFile(name, nextCluster(firstCluster));
}

    

//     struct mbr_partition_entry *p1 = (typeof(p1)) (&buffer[446]);
//     assert(p1->type == 0x0C); // FAT32
//     lba_first = p1->lba_first;
//     readSector(buffer, lba_first);

//     struct fat32_header *header = (typeof(header)) buffer;
//     assert(header->bytes_per_sector == SECTOR);
//     sectors_per_cluster = header->sectors_per_cluster;

//     fat_begin = lba_first + header->reserved_sectors;
//     cluster_begin = lba_first + header->reserved_sectors + header->fat_count * header->sectors_per_fat;
//     assert(header->root_cluster == 2);

    

//     struct fat32_directory_entry weights = findFile("GEMMA   SBS", header->root_cluster);
//     printk("weights->name: %c%c%c%c%c%c%c%c%c%c%c\n", weights.name[0], weights.name[1], weights.name[2], weights.name[3], weights.name[4], weights.name[5], weights.name[6], weights.name[7], weights.name[8], weights.name[9], weights.name[10]);
//     printk("weights->size: %u GB\n", (uint32_t) weights.size / 1000 / 1000 / 1000);
//     printk("weights->address: %x\n", CLUSTER_TO_SECTOR(ENTRY_CLUSTER(weights)));
//     readSector(buffer, CLUSTER_TO_SECTOR(ENTRY_CLUSTER(weights)));
//     printk("First 8 bytes:\n%c%c%c%c%c%c%c%c\n", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6], buffer[7]);

//     return 0;
// }



void notmain() {
    printk("Starting SD Driver test.\n");

    printk("Initializing...\n");
    emmc_init();

    printk("Reading first block...\n");

    uint8_t buffer[SECTOR];
    readSector(buffer, 0); // MBR    
    assert(buffer[SECTOR - 2] == 0x55 && buffer[SECTOR - 1] == 0xAA);

    struct mbr_partition_entry *p1 = (typeof(p1)) (&buffer[446]);
    assert(p1->type == 0x0C); // FAT32
    lba_first = p1->lba_first;
    readSector(buffer, lba_first);

    struct fat32_header *header = (typeof(header)) buffer;
    assert(header->bytes_per_sector == SECTOR);
    sectors_per_cluster = header->sectors_per_cluster;

    fat_begin = lba_first + header->reserved_sectors;
    cluster_begin = lba_first + header->reserved_sectors + header->fat_count * header->sectors_per_fat;
    assert(header->root_cluster == 2);

    struct fat32_directory_entry weights = findFile("GEMMA   SBS", header->root_cluster);
    printk("weights->name: %c%c%c%c%c%c%c%c%c%c%c\n", weights.name[0], weights.name[1], weights.name[2], weights.name[3], weights.name[4], weights.name[5], weights.name[6], weights.name[7], weights.name[8], weights.name[9], weights.name[10]);
    printk("weights->size: %u GB\n", (uint32_t) weights.size / 1000 / 1000 / 1000);
    printk("weights->address: %x\n", CLUSTER_TO_SECTOR(ENTRY_CLUSTER(weights)));
    readSector(buffer, CLUSTER_TO_SECTOR(ENTRY_CLUSTER(weights)));
    printk("First 8 bytes:\n%c%c%c%c%c%c%c%c\n", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6], buffer[7]);
    
    //6000th byte:
    uint32_t byte_to_read = 60000;
    uint32_t cluster = ENTRY_CLUSTER(weights);
    for (int i = 0; i < byte_to_read / (SECTOR * sectors_per_cluster); i++) {
        cluster = nextCluster(cluster);
    }
    readSector(buffer, CLUSTER_TO_SECTOR(cluster));
    printk("60000th byte: %c\n", buffer[byte_to_read % (SECTOR * sectors_per_cluster)]);

    printk("Success!: %s\n", __FILE__);
    clean_reboot();
}
