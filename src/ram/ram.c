//
// Created by quate on 2/15/2024.
//

#include <stdlib.h>
#include <stdio.h>
#include "ram.h"
#include "../ppu.h"

#define INTERNAL_RAM 0x1FFF
#define RAM_MASK 0x7FF
#define PPU 0x3FFF
#define PPU_REG_MASK 0x7
#define CARTRIDGE_ADDR_OFFSET 0x4000

uint8_t ram[RAM_SIZE];
uint8_t cartridge[CARTRIDGE_SIZE];  /// includes apu/io
uint16_t addr_bus = 0;
uint8_t data_bus = 0;

/**
 * Returns a pointer to the byte located at the provided memory address.
 *
 * The NES memory map has the following layout:
 * 0x0000-0x07FF: Internal RAM
 * 0x0800-0x1FFF: Mirrors of 0x0000-0x07FF
 * 0x2000-0x2007: CPU view of PPU registers
 * 0x2008-0x3FFF: Mirrors of 0x2000-0x2007
 * 0x4000-0xFFFF: Device and cartridge space (exact layout and memory usage depends on the cartridge)
 *
 * @param addr The 16-bit address to look up.
 * @return A pointer to the byte. Returns NULL if the address is invalid.
 */
uint8_t* map(uint16_t addr)
{
    if ((addr & INTERNAL_RAM) == addr) {  // 000....
        return &ram[addr & RAM_MASK];
    } else if ((addr | PPU) == PPU) {  // 001....
        return &((uint8_t*) &ppu_registers)[addr & PPU_REG_MASK];
    } else {
        return &cartridge[addr - CARTRIDGE_ADDR_OFFSET];
    }
}


void read()
{
    if (map(addr_bus) != NULL) {
        data_bus = *map(addr_bus);
    }
    // Default open bus
}

void write()
{
    if (map(addr_bus) != NULL) {
        *map(addr_bus) = data_bus;
    }
    // Default open bus
}
