//
// Created by quate on 2/15/2024.
//

#include "ram.h"
#define INTERNAL_RAM 0x2000
#define RAM_MASK 0x7FF

uint8_t read(uint16_t addr)
{
    if (addr < INTERNAL_RAM) {
        return ram[addr & RAM_MASK];
    } else {
        return -1;  // PPU and other things
    }
}

void write(uint16_t addr, uint8_t val)
{

}
