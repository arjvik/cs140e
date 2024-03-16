PROGS = uart-server.c uart-client.c

# uncomment if you want it to automatically run.
RUN=0

LIBS += $(CS140E_2024_PATH)/lib/libgcc.a

BOOTLOADER = my-install
EXCLUDE ?= grep -v simple_boot
include $(CS140E_2024_PATH)/libpi/mk/Makefile.robust

clean::
	make -C tests clean