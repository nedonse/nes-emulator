//
// Created by quate on 2/17/2024.
//

#ifndef TINY_EMULATOR_LOAD_H
#define TINY_EMULATOR_LOAD_H

#include <stdio.h>
#include <stdint.h>

/**
 * Allocates array of cartridge data into memory.
 * Only designed to work with cartridges with no mapper (bank switching).
 *
 * @param file_path
 * @return
 */
uint8_t* load_rom(const char* file_path);

#endif //TINY_EMULATOR_LOAD_H
