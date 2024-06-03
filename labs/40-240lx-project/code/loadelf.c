#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
// #include <assert.h>
#define assert(x) if (!(x)) { panic("Assertion failed: %s\n", #x); }
#include "rpi.h"
#include "fat32-arjun.h"
#include "memmap.h"
#include "full-except.h"
#include "breakpoint.h"

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

void elf32_check_header(struct elf32_header *header) {
    assert(header->ident.magic == 0x464c457f);
    assert(header->ident.class == 1);
    assert(header->ident.data == 1);
    assert(header->ident.version == 1);
    assert(header->ident.osabi == 0);
    assert(header->ident.abiversion == 0);
    assert(header->type == 2);
    assert(header->machine == 40);
    assert(header->version == 1);
    assert(header->header_size == 52);
    assert(header->program_header_entry_size == 32);
    assert(header->section_header_entry_size == 40);
}

static int syscall_handler(regs_t *r) {
    unsigned *pc = (unsigned *)(r->regs[REGS_PC] - 4);
    unsigned immediate = *pc & ((1<<24)-1);
    assert(immediate == 0);
    unsigned sysno = r->regs[7];


    printk("pc %x, instr %x %x (%x) %x %x\n", pc, *(pc-2), *(pc-1), *pc, *(pc+1), *(pc+2));
    // printk("pc %x, instr %x %x (%x) %x %x\n", pc, *(pc-2), *(pc-1), *pc, *(pc+1), *(pc+2));
    // assert(sysno == 0); // stupid GCC, should be 128
    // printk("   handling syscall %d\n", sysno);
    printk("SWI Interrupt at %x - from %s\n", pc, sysno, mode_get(cpsr_get()) == USER_MODE ? "user mode" : "super mode");
    printk("r0: %x, r1: %x, r2: %x, r3: %x, r7: %x\n", r->regs[0], r->regs[1], r->regs[2], r->regs[3], r->regs[7]);
    // printk("r0: %x, r1: %x, r2: %x, r3: %x, r4: %x, r5: %x, r6: %x, r7: %x\n", r->regs[0], r->regs[1], r->regs[2], r->regs[3], r->regs[4], r->regs[5], r->regs[6], r->regs[7]);
    
    if (sysno == 1) {
        printk("Exiting!\n");
        clean_reboot();
    } else if (sysno == 4) {
        printk("Writing `%s`!\n", (char *)r->regs[1]);
        return 1;
    } else {
        return 0;
    }
}

static void single_step_handler_full(regs_t *r) {
    if(!brkpt_fault_p())
        panic("impossible: should get no other faults\n");

    unsigned pc = r->regs[REGS_PC];
    brkpt_mismatch_set(pc);
    printk("single-step handler: pc=%x instr=%x\n", pc, *(unsigned *)(pc));

    brkpt_mismatch_set(pc);
    switchto(r);
}

void notmain() {
    full_except_install(0);
    full_except_set_syscall(syscall_handler);
    full_except_set_prefetch(single_step_handler_full);
    
    uint32_t cpsr_cur = cpsr_get();
    assert(mode_get(cpsr_cur) == SUPER_MODE);
    uint32_t cpsr_A = mode_set(cpsr_cur, USER_MODE);


    struct fat32_filesystem fs = fat32init(1);
    uint8_t buffer[SECTOR];

    struct fat32_directory_entry binary;
    // assert(findFile(&fs, "NOLIBHWD   ", fs.root_cluster, &binary));
    assert(traverseToFile(&fs, (const char *[]){"bin", "hello"}, 2, fs.root_cluster, &binary));

    uint8_t *binary_buffer = kmalloc(((binary.size + SECTOR - 1) / SECTOR) * SECTOR);
    uint32_t binary_cluster = ENTRY_CLUSTER(binary);
    int i = 0;
    while (i < binary.size) {
        readSector(&binary_buffer[i], CLUSTER_TO_SECTOR(fs, binary_cluster) + (i / SECTOR) % fs.sectors_per_cluster);
        i += SECTOR;
        if (i % (SECTOR * fs.sectors_per_cluster) == 0)
            binary_cluster = nextCluster(&fs, binary_cluster);
    }

    struct elf32_header *header = (struct elf32_header *)binary_buffer;
    elf32_check_header(header);

    printk("%d section headers, %d program headers\n", header->section_header_entry_count, header->program_header_entry_count);

    printk("Important addresses:\n");
    extern uint32_t full_except_ints[];
    printk("__code_start__=%x, __prog_end__=%x\n", __code_start__, __prog_end__);
    printk("full_except_ints=%x\n", (void *) full_except_ints);
    printk("syscall_handler=%x\n", syscall_handler);

    unsigned max_elf_address = 0;

    for (unsigned i = 0; i < header->program_header_entry_count; i++) {
        struct elf32_program_header *program = (struct elf32_program_header *)(binary_buffer + header->program_header_offset + i * header->program_header_entry_size);
        // if (program->type != 1 && program->type != 7) continue;
        printk("Segment %d: %x - %x\n", i, program->virtual_address, program->virtual_address + program->memory_size);
        printk("  type: %x\n", program->type);
        printk("  offset: %x\n", program->offset);
        printk("  virtual address: %x\n", program->virtual_address);
        printk("  physical address: %x\n", program->physical_address);
        printk("  file size: %x\n", program->file_size);
        printk("  memory size: %x\n", program->memory_size);
        printk("  flags: %x\n", program->flags);
        printk("  alignment: %x\n", program->alignment);
        
        if (program->virtual_address + program->memory_size > max_elf_address) {
            max_elf_address = program->virtual_address + program->memory_size;
        }
        
        // check if the segment's memory size overlaps with __code_start__ and __prog_end__
        if (program->virtual_address < (uint32_t)__prog_end__ && program->virtual_address + program->memory_size > (uint32_t)__code_start__) {
            panic("  !!! OVERLAPS with __code_start__=%x\n, __prog_end__=%x !!\n", __code_start__, __prog_end__);
        }
        // copy file size bytes into memory at virtual address, zeroing out the rest of memory size
        memcpy((void *)program->virtual_address, binary_buffer + program->offset, program->file_size);
        memset((void *)(program->virtual_address + program->file_size), 0, program->memory_size - program->file_size);
    }
    max_elf_address = (max_elf_address + 0xfffff) & ~0xfffff;
    unsigned heap = max_elf_address;
    unsigned stack_start = heap + 0x100000;
    unsigned stack_end = stack_start + 0x100000;
    unsigned stack = stack_end - 0x1000;

    // zero out heap and stack
    memset((void *)heap, 0, stack_end - heap);

    strcpy((void *)stack_end, "HELLOWD");

    // fill stack
    volatile unsigned *stack_ptr = (volatile void *)stack;
    *(stack_ptr++) = 1;
    *(stack_ptr++) = stack_end;
    *(stack_ptr++) = 0;
    *(stack_ptr++) = 0;

    printk("First sixteen instructions at entrypoint:\n");
    for (unsigned i = 0; i < 16; i++)
        printk("  %x: %x\n", header->entry_address + i * 4, *(uint32_t *)(header->entry_address + i * 4));

    // jump to entry point with stack pointer set
    printk("Jumping to entry point %x with stack pointer %x, heap pointer %x\n", header->entry_address, stack, heap);
    assert(header->entry_address % sizeof(unsigned) == 0);

    asm volatile (
        "mov sp, %0\n"
        "mov pc, %1\n"
        :
        : "r" (stack), "r" (header->entry_address)
    );

    printk("Success!: %s\n", __FILE__);
    clean_reboot();
}