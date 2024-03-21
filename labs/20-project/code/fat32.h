#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
// #include <assert.h>
// #define assert(x) if (!(x)) { panic("Assertion failed: %s\n", #x); }
#include "rpi.h"

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

#define ARRAY_SIZEOF_MEMBER(type, member) (sizeof(((type *)NULL)->member) / sizeof(*(((type *)NULL)->member)))
#define CHARS_PER_LFN_ENTRY (ARRAY_SIZEOF_MEMBER(struct fat32_lfn_entry, name) + ARRAY_SIZEOF_MEMBER(struct fat32_lfn_entry, name2) + ARRAY_SIZEOF_MEMBER(struct fat32_lfn_entry, name3))
_Static_assert(CHARS_PER_LFN_ENTRY == 13);

#define OUT
bool findFile(const char *name, uint32_t firstCluster, OUT struct fat32_directory_entry *result);

bool traverseToFile(const char **name, size_t segments, uint32_t firstCluster, OUT struct fat32_directory_entry *result);

void readEntireFile(uint8_t *dst, const struct fat32_directory_entry file);

void fat32init(void);