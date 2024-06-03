CC := arm-linux-gnueabi-gcc
CFLAGS := -march=armv6 -nostdlib

COMMON_SRC := start.S

all: $(patsubst %.c, %, $(wildcard *.c))

%: %.c
	$(CC) $(CFLAGS) $(COMMON_SRC) $< -o $@