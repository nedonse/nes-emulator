//
// Created by quate on 2/17/2024.
//

#ifndef TINY_EMULATOR_LOAD_H
#define TINY_EMULATOR_LOAD_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

struct nes_file
{
    /// Vertical mirroring? Otherwise horizontal
//    bool v_mirror;
    /// 12-bit index for mapper behavior class
    uint16_t mapper_idx;
    size_t prg_size;
    size_t chr_size;
    size_t data_size;
    uint8_t* data;
};

/**
 * Allocates array of cartridge data into memory.
 * Only designed to work with cartridges with no mapper (bank switching).
 *
 * @param file_path
 * @return
 */
struct nes_file open_file(const char* file_path);

void load_file(const struct nes_file* file);

#endif //TINY_EMULATOR_LOAD_H
