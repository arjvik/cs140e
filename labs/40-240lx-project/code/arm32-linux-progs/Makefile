CC := arm-linux-gnueabi-gcc
CFLAGS := -march=armv6 -nostdlib -g

COMMON_SRC := start.S
COMMON_HDR := $(wildcard *.h)

all: $(patsubst %.c, %, $(wildcard *.c))

%: %.c $(COMMON_SRC) $(COMMON_HDR)
	$(CC) $(CFLAGS) $(COMMON_SRC) $< -o $@

clean:
	rm -f $(patsubst %.c, %, $(wildcard *.c))

.PHONY: all clean