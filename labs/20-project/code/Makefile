COMMON_SRC = parallel.c vm-4k-pages.c emmc.c fat32.c staff-mmu.c pt-vm.c staff-full-except.c full-except-asm.S
PROGS = $(wildcard tests/teleport-*.c)

RUN=0

STAFF_OBJS += $(CS140E_2024_PATH)/labs/16-vm-page-table/code/staff-pt-vm.o
STAFF_OBJS += $(CS140E_2024_PATH)/labs/16-vm-page-table/code/staff-mmu.o
STAFF_OBJS += $(CS140E_2024_PATH)/labs/16-vm-page-table/code/staff-mmu-asm.o
STAFF_OBJS += $(CS140E_2024_PATH)/libpi/staff-objs/staff-switchto-asm.o


LIBS += $(CS140E_2024_PATH)/lib/libgcc.a

BOOTLOADER = my-install
EXCLUDE ?= grep -v simple_boot
include $(CS140E_2024_PATH)/libpi/mk/Makefile.robust