//
// Created by quate on 2/17/2024.
//

#ifndef TINY_EMULATOR_LOAD_H
#define TINY_EMULATOR_LOAD_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "cartridge/ines.h"


/**
 * Allocates array of cartridge data into memory.
 * Only designed to work with cartridges with no mapper (bank switching).
 *
 * @param file_path
 * @return
 */
struct nes_file open_file(const char* file_path);

void load_file(struct nes_file* file);

#endif //TINY_EMULATOR_LOAD_H
