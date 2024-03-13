#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
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
_Static_assert(sizeof(struct mbr_partition_entry) == 16);

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
    struct fat32_directory_entry_attributes {
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
_Static_assert(sizeof(struct fat32_directory_entry) == 32);

struct fat32_lfn_entry {
    uint8_t sequence;
    uint16_t name[5];
    struct fat32_directory_entry_attributes attributes;
    uint8_t type;
    uint8_t checksum;
    uint16_t name2[6];
    uint16_t first_cluster_fake;
    uint16_t name3[2];
} __attribute__((packed));
_Static_assert(sizeof(struct fat32_lfn_entry) == sizeof(struct fat32_directory_entry));
_Static_assert(offsetof(struct fat32_lfn_entry, first_cluster_fake) == offsetof(struct fat32_directory_entry, first_cluster_low));

#define ENTRY_CLUSTER(e) ((((uint32_t) (e).first_cluster_high) << 16) | ((uint32_t) (e).first_cluster_low))
#define ARRAY_SIZEOF_MEMBER(type, member) (sizeof(((type *)NULL)->member) / sizeof(*(((type *)NULL)->member)))
#define CHARS_PER_LFN_ENTRY (ARRAY_SIZEOF_MEMBER(struct fat32_lfn_entry, name) + ARRAY_SIZEOF_MEMBER(struct fat32_lfn_entry, name2) + ARRAY_SIZEOF_MEMBER(struct fat32_lfn_entry, name3))
_Static_assert(CHARS_PER_LFN_ENTRY == 13);

FILE *file;
uint32_t sectors_per_cluster;
uint32_t lba_first;
uint32_t cluster_begin;
uint32_t fat_begin;
#define CLUSTER_TO_SECTOR(c) (cluster_begin + ((c) - 2) * sectors_per_cluster)

void readSector(uint8_t *buffer, uint32_t sector) {
    emmc_read(sector, buffer, 512);
}

uint32_t nextCluster(uint32_t c) {
#define FAT_ENTRIES_PER_SECTOR (SECTOR / sizeof(uint32_t))
    uint8_t buffer[SECTOR];
    readSector(buffer, fat_begin + c / FAT_ENTRIES_PER_SECTOR);
    return ((uint32_t *) buffer)[c % FAT_ENTRIES_PER_SECTOR];
}

#define MAX_NUM_LFN_ENTRIES 4

void readLFN(char *dst, uint8_t seq, const uint8_t name[restrict static 10], const uint8_t name2[restrict static 12], const uint8_t name3[restrict static 4]) {
    assert((seq & ~0x40) <= MAX_NUM_LFN_ENTRIES); 
    uint8_t entry_offset = ((seq & ~0x40) - 1) * CHARS_PER_LFN_ENTRY;
    uint8_t name_offset = 0;
    for (int i = 0; i < ARRAY_SIZEOF_MEMBER(struct fat32_lfn_entry, name); i++) {
        dst[entry_offset + name_offset + i] = name[i << 1];
        if (!name[i << 1]) {
            assert(seq & 0x40);
            return;
        }
    }
    name_offset += ARRAY_SIZEOF_MEMBER(struct fat32_lfn_entry, name);
    for (int i = 0; i < ARRAY_SIZEOF_MEMBER(struct fat32_lfn_entry, name2); i++) {
        dst[entry_offset + name_offset + i] = name2[i << 1];
        if (!name2[i << 1]) {
            assert(seq & 0x40);
            return;
        }
    }
    name_offset += ARRAY_SIZEOF_MEMBER(struct fat32_lfn_entry, name2);
    for (int i = 0; i < ARRAY_SIZEOF_MEMBER(struct fat32_lfn_entry, name3); i++) {
        dst[entry_offset + name_offset + i] = name3[i << 1];
        if (!name3[i << 1]) {
            assert(seq & 0x40);
            return;
        }
    }
    name_offset += ARRAY_SIZEOF_MEMBER(struct fat32_lfn_entry, name3);
    if (seq & 0x40)
        dst[entry_offset + name_offset] = 0;
}

#define OUT
bool findFile(const char *name, uint32_t firstCluster, OUT struct fat32_directory_entry *result) {
    uint8_t buffer[SECTOR];
    uint8_t lfn_buffer[MAX_NUM_LFN_ENTRIES * CHARS_PER_LFN_ENTRY];
    bool lfnNext = false;
    for (int sector_idx = 0; sector_idx < sectors_per_cluster; sector_idx++) {
        readSector(buffer, CLUSTER_TO_SECTOR(firstCluster) + sector_idx);
        struct fat32_directory_entry *dir = (typeof(dir)) buffer;
        for (int i = 0; i < SECTOR / sizeof(*dir); i++) {
            // assert (dir[i].name[0] != 0x00);
            if (dir[i].name[0] == 0x00) return false;
            if (dir[i].name[0] == 0xE5) continue;
            if (dir[i].attributes.read_only && dir[i].attributes.hidden && dir[i].attributes.system && dir[i].attributes.volume_id) {
                struct fat32_lfn_entry *lfn = (typeof(lfn)) &dir[i];
                readLFN(lfn_buffer, lfn->sequence, (uint8_t *) lfn->name, (uint8_t *) lfn->name2, (uint8_t *) lfn->name3);
                if ((lfn->sequence & ~0x40) == 1)
                    lfnNext = true;
                continue;
            }
            bool lfnCurrent = lfnNext;
            lfnNext = false;

            if (lfnCurrent)
            {
                int j = 0;
                while (name[j] == lfn_buffer[j] && name[j])
                    j++;
                if (name[j] == 0)
                {
                    *result = dir[i];
                    return true;
                }
            }
            int k;
            for (k = 0; k < 11; k++)
                if (dir[i].name[k] != name[k])
                    break;
            if (k == 11) {
                *result = dir[i];
                return true;
            }
        }
    }
    return findFile(name, nextCluster(firstCluster), result);
}

bool traverseToFile(const char **name, size_t segments, uint32_t firstCluster, OUT struct fat32_directory_entry *result) {
    while (segments > 1) {
        if (!findFile(*name, firstCluster, result)) {
            return false;
        }
        assert(result->attributes.directory);
        firstCluster = ENTRY_CLUSTER(*result);
        name++;
        segments--;
    }
    assert(segments == 1);
    return findFile(*name, firstCluster, result);
}

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

    fat_begin = lba_first + header->reserved_sectors;
    cluster_begin = lba_first + header->reserved_sectors + header->fat_count * header->sectors_per_fat;
    uint32_t root_cluster = header->root_cluster;
    assert(root_cluster == 2);

    printk("Reading 'gemma.sbs':\n");
    struct fat32_directory_entry weights;
    assert(findFile("gemma.sbs", root_cluster, &weights));
    printk("weights->name: %c%c%c%c%c%c%c%c%c%c%c\n", weights.name[0], weights.name[1], weights.name[2], weights.name[3], weights.name[4], weights.name[5], weights.name[6], weights.name[7], weights.name[8], weights.name[9], weights.name[10]);
    printk("weights->size: %u GB\n", (uint32_t) weights.size / 1000 / 1000 / 1000);
    printk("weights->address: %x\n", CLUSTER_TO_SECTOR(ENTRY_CLUSTER(weights)));
    readSector(buffer, CLUSTER_TO_SECTOR(ENTRY_CLUSTER(weights)));
    printk("First 8 bytes:\n%c%c%c%c%c%c%c%c\n", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6], buffer[7]);

    printk("\nReading 'testfolder/asuperdupermegalongfilename.txt':\n");
    struct fat32_directory_entry longfile;
    assert(traverseToFile((const char *[]){"testfolder", "asuperdupermegalongfilename.txt"}, 2, root_cluster, &longfile));
    printk("longfile->name: %c%c%c%c%c%c%c%c%c%c%c\n", longfile.name[0], longfile.name[1], longfile.name[2], longfile.name[3], longfile.name[4], longfile.name[5], longfile.name[6], longfile.name[7], longfile.name[8], longfile.name[9], longfile.name[10]);
    printk("longfile->size: %u bytes\n", (uint32_t) longfile.size);
    printk("longfile->address: %x\n", CLUSTER_TO_SECTOR(ENTRY_CLUSTER(longfile)));
    readSector(buffer, CLUSTER_TO_SECTOR(ENTRY_CLUSTER(longfile)));
    printk("First 8 bytes:\n%c%c%c%c%c%c%c%c\n", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6], buffer[7]);

    printk("Loading binary 'hello-f.bin':\n");
    struct fat32_directory_entry binary;
    assert(findFile("hello-f.bin", root_cluster, &binary));
    assert(binary.size < SECTOR * sectors_per_cluster);
    uint8_t binary_buffer[((binary.size + SECTOR - 1) / SECTOR) * SECTOR];
    uint32_t binary_cluster = ENTRY_CLUSTER(binary);
    int i = 0;
    while (i < binary.size) {
        readSector(&binary_buffer[i], CLUSTER_TO_SECTOR(binary_cluster) + i / SECTOR);
        i += SECTOR;
        if (i % (SECTOR * sectors_per_cluster) == 0)
            binary_cluster = nextCluster(binary_cluster);
    }
    size_t load_addr = 0x90000;
    for (i = 0; i < binary.size; i++)
        PUT8(load_addr + i, binary_buffer[i]);
    
    ((void (*)()) binary_buffer)();

    printk("Success!: %s\n", __FILE__);
    clean_reboot();
}