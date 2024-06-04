#include "elf32.h"
#include "memmap.h"
#include "rpi.h"
// #define assert(x) if (!(x)) { panic("Assertion failed: %s\n", #x); }

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

void run_elf32(void *elf32_buffer, int argc, const char *argv[]) {

    struct elf32_header *header = (struct elf32_header *) elf32_buffer;
    elf32_check_header(header);

    unsigned max_elf_address = 0;

    for (unsigned i = 0; i < header->program_header_entry_count; i++) {
        struct elf32_program_header *program = (struct elf32_program_header *)(elf32_buffer + header->program_header_offset + i * header->program_header_entry_size);
        if (program->type != 1 && program->type != 7) continue;

        if (program->virtual_address + program->memory_size > max_elf_address)
            max_elf_address = program->virtual_address + program->memory_size;
        
        if (program->virtual_address < (uint32_t)__prog_end__ && program->virtual_address + program->memory_size > (uint32_t)__code_start__)
            panic("\n!!! OVERLAPS with __code_start__=%x, __prog_end__=%x\n  !!! !!! virtual_address=%x, virtual_end=%x\n", __code_start__, __prog_end__, program->virtual_address, program->virtual_address + program->memory_size);
        
        memcpy((void *)program->virtual_address, elf32_buffer + program->offset, program->file_size);
        memset((void *)(program->virtual_address + program->file_size), 0, program->memory_size - program->file_size);
    }

    max_elf_address = (max_elf_address + 0xfffff) & ~0xfffff;
    unsigned heap = max_elf_address;
    unsigned stack_start = heap + 0x100000;
    unsigned stack_end = stack_start + 0x100000;
    unsigned stack = stack_end - 0x1000;

    // zero out heap and stack
    memset((void *)heap, 0, stack_end - heap);

    // // argv at end of stack
    // strcpy((void *)stack_end, program_name);

    // // fill stack
    // volatile unsigned *stack_ptr = (volatile void *)stack;
    // *(stack_ptr++) = 1;
    // *(stack_ptr++) = stack_end;
    // *(stack_ptr++) = 0;
    // *(stack_ptr++) = 0;

    // fill stack with argc and argv
    volatile unsigned *stack_ptr = (volatile void *)stack;
    volatile unsigned *stack_end_ptr = (volatile void *)stack_end;
    *(stack_ptr++) = argc;
    for (int i = 0; i < argc; i++) {
        *(stack_ptr++) = (unsigned)stack_end_ptr;
        strcpy((void *)stack_end_ptr, argv[i]);
        stack_end_ptr += strlen(argv[i]) + 1;
    }
    *(stack_ptr++) = 0; // null-terminate argv
    *(stack_ptr++) = 0; // null-terminate empty envp

    assert(header->entry_address % sizeof(unsigned) == 0);

    // jump to entry point with stack pointer set
    asm volatile (
        "mov sp, %0\n"
        "mov pc, %1\n"
        :
        : "r" (stack), "r" (header->entry_address)
    );
}