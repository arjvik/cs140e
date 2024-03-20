#include <stddef.h>
#include "rpi.h"

void parallel_setup_read(void);

uint8_t parallel_read_byte(void);

uint32_t parallel_read_32(void);

void parallel_read_n(uint32_t *data, size_t n);

void parallel_setup_write(void);

void parallel_write_byte(uint8_t byte);

void parallel_write_32(uint32_t x);

void parallel_write_n(const uint32_t *data, size_t n);