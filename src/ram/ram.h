//
// Created by quate on 2/14/2024.
//

#ifndef NES_EMULATOR__RAM_H
#define NES_EMULATOR__RAM_H

#include <stdint.h>
#include <stdbool.h>

#define RAM_SIZE 0x800  // 2 KB
#define PPU_REG_SIZE 0x8
#define CARTRIDGE_SIZE 0xC000  /// includes apu/io

extern uint8_t ram[RAM_SIZE];
extern uint8_t cartridge[CARTRIDGE_SIZE];  /// includes apu/io
extern uint16_t addr_bus;
extern uint8_t data_bus;

/// Sends read signal to memory bus
void read();

/// Sends write signal to memory bus
void write();

#endif //NES_EMULATOR__RAM_H
