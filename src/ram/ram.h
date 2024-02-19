//
// Created by quate on 2/14/2024.
//

#ifndef TINY_EMULATOR_RAM_H
#define TINY_EMULATOR_RAM_H

#include <stdint.h>

#define RAM_SIZE 0x800  // 2 KB
#define PPU_REG_SIZE 0x8

static uint8_t ram[RAM_SIZE];
static uint8_t ppu[PPU_REG_SIZE];
static uint8_t bus_value = 0;

/// Sends read signal to memory bus
/// bus_value will be set to the retrieved value by the next cycle
void read(uint16_t addr);

/// Sends write signal to memory bus
void write(uint16_t addr);

#endif //TINY_EMULATOR_RAM_H
