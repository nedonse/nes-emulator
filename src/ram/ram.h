//
// Created by quate on 2/14/2024.
//

#ifndef TINY_EMULATOR_RAM_H
#define TINY_EMULATOR_RAM_H

#include <stdint.h>

#define RAM_SIZE 0x800  // 2 KB

static uint8_t ram[RAM_SIZE];

uint8_t read(uint16_t addr);
void write(uint16_t addr, uint8_t val);

#endif //TINY_EMULATOR_RAM_H
