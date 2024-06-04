#include <stdint.h>

struct elf32_header_ident {
    uint32_t magic; // should be 0x7f 'ELF'
    uint8_t class; // should be 1 for 32bit
    uint8_t data; // should be 1 for LE
    uint8_t version; // should be 1
    uint8_t osabi; // should in fact be 0
    uint8_t abiversion; // should be 0
    uint8_t pad[7];
};
_Static_assert(sizeof(struct elf32_header_ident) == 16);

struct elf32_header {
    struct elf32_header_ident ident;
    uint16_t type; // should be 2 for EXEC
    uint16_t machine; // should be 40 for ARM
    uint32_t version; // should be 1
    uint32_t entry_address;
    uint32_t program_header_offset;
    uint32_t section_header_offset;
    uint32_t flags;
    uint16_t header_size; // should be 52
    uint16_t program_header_entry_size; // should be 32
    uint16_t program_header_entry_count;
    uint16_t section_header_entry_size; // should be 40
    uint16_t section_header_entry_count;
    uint16_t section_header_string_table_index;
};
_Static_assert(sizeof(struct elf32_header) == 52);

struct elf32_program_header {
    uint32_t type;
    uint32_t offset;
    uint32_t virtual_address;
    uint32_t physical_address;
    uint32_t file_size;
    uint32_t memory_size;
    uint32_t flags;
    uint32_t alignment;
};
_Static_assert(sizeof(struct elf32_program_header) == 32);

struct elf32_section_header {
    uint32_t name;
    uint32_t type;
    uint32_t flags;
    uint32_t address;
    uint32_t offset;
    uint32_t size;
    uint32_t link;
    uint32_t info;
    uint32_t alignment;
    uint32_t entry_size;
};
_Static_assert(sizeof(struct elf32_section_header) == 40);

enum elf32_section_header_type {
    SHT_NULL = 0,
    SHT_PROGBITS = 1,
    SHT_SYMTAB = 2,
    SHT_STRTAB = 3,
    SHT_RELA = 4,
    SHT_NOBITS = 8,
    SHT_REL = 9,
};

enum elf32_section_header_flags {
    SHF_WRITE = 0x1,
    SHF_ALLOC = 0x2,
    SHF_EXECINSTR = 0x4,
};

void run_elf32(void *elf32_buffer, int argc, const char *argv[]);