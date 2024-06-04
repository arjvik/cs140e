COMMON_SRC = staff-full-except.c full-except-asm.S emmc.c fat32-arjun.c elf32.c syscall-handler.c longjmp.c

PROGS = shell.c

RUN=1

STAFF_OBJS += staff-switchto-asm.o staff-single-step.o


LIBS += $(CS140E_2024_PATH)/lib/libgcc.a

# BOOTLOADER = my-install
BOOTLOADER = pi-install
EXCLUDE ?= grep -v simple_boot
include $(CS140E_2024_PATH)/libpi/mk/Makefile.robust