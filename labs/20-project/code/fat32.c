#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
// #include <assert.h>
// #define assert(x) if (!(x)) { panic("Assertion failed: %s\n", #x); }
#include "rpi.h"
#include "emmc.h"
#include "fat32.h"

#define ENTRY_CLUSTER(e) ((((uint32_t) (e).first_cluster_high) << 16) | ((uint32_t) (e).first_cluster_low))

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

void readEntireFile(uint8_t *dst, const struct fat32_directory_entry file) {
    // printk("Loading binary 'hello-f.bin':\n");
    // struct fat32_directory_entry binary;
    // assert(findFile("hello-f.bin", 2, &binary));
    // assert(file.size < SECTOR * sectors_per_cluster);
    // uint8_t binary_buffer[((file.size + SECTOR - 1) / SECTOR) * SECTOR];
    uint32_t binary_cluster = ENTRY_CLUSTER(file);
    int i = 0;
    while (i < file.size) {
        readSector(&dst[i], CLUSTER_TO_SECTOR(binary_cluster) + i / SECTOR);
        i += SECTOR;
        if (i % (SECTOR * sectors_per_cluster) == 0)
            binary_cluster = nextCluster(binary_cluster);
    }
}

void fat32init(void) {
    emmc_init();

    uint8_t buffer[SECTOR];
    readSector(buffer, 0); // MBR    
    assert(buffer[SECTOR - 2] == 0x55 && buffer[SECTOR - 1] == 0xAA);

    struct mbr_partition_entry *p1 = (typeof(p1)) (&buffer[446]);
    assert(p1->type == 0x0C || p1->type == 0x0B); // FAT32
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
}