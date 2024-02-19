//
// Created by quate on 2/15/2024.
//

#include "ram.h"
#define INTERNAL_RAM 0x1FFF
#define RAM_MASK 0x7FF
#define PPU 0x3FFF
#define APU_IO 0x401F
#define PPU_REG_MASK 0x7


uint8_t* map(uint16_t addr)
{
    if ((addr & INTERNAL_RAM) == addr) {  // 000....
        return &ram[addr & RAM_MASK];
    } else if ((addr | PPU) == PPU) {  // 001....
        return &ppu[addr & PPU_REG_MASK];
    } else if ((addr | APU_IO) == APU_IO) {  // 01000000_00011111
        return NULL;  // TODO
    } else {
        return NULL;
    }
}

void read(uint16_t addr)
{
    if (map(addr) != NULL) {
        bus_value = *map(addr);
    }
    // Default open bus
}

void write(uint16_t addr)
{
    if (map(addr) != NULL) {
        *map(addr) = bus_value;
    }
    // Default open bus
}
