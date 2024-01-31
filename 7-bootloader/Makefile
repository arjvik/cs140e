all: my-install
clean:
	rm my-install
my-install: my-install.c boot-protocol.h
	gcc -o my-install my-install.c
kernel.img: bootloader.c boot-protocol.h
	arm-none-eabi-gcc -march=armv6 -o bootloader bootloader.c
	arm-none-eabi-objcopy bootloader -O binary kernel.img
	rm bootloader
.PHONY: all clean