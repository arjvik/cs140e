// *(unsigned *)addr = v;
void PUT32(unsigned addr, unsigned v);
void put32(volatile void *addr, unsigned v);

// *(unsigned *)addr
unsigned GET32(unsigned addr);
unsigned get32(const volatile void *addr);

// *(volatile uint8_t *)addr = x;
void put8(volatile void *addr, uint8_t x);
void PUT8(uint32_t addr, uint8_t x);

uint8_t GET8(unsigned addr);
uint8_t get8(const volatile void *addr);

// jump to <addr>
void BRANCHTO(unsigned addr);


int notmain() {
    
}