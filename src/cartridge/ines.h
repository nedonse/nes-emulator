//
// Created by quate on 3/25/2024.
//

#ifndef NES_EMULATOR_INES_H
#define NES_EMULATOR_INES_H

#include <stdint.h>
#include <stdlib.h>

#define NES_FILE_HEADER_SIZE 16

#define PRG_PAGE_SIZE 0x4000  // 16 kB
#define CHR_PAGE_SIZE 0x2000  // 8 kB

struct nes_file
{
    /// Vertical mirroring? Otherwise horizontal
//    bool v_mirror;
    /// 12-bit index for mapper behavior class
    uint16_t mapper_idx;

    /// PRG size in units of 16 kB
    size_t prg_size;

    /// CHR size in units of 8 kB
    size_t chr_size;

    uint8_t* prg_rom;
    uint8_t* chr_rom;
};

static size_t get_prg_size_bytes(size_t prg_size) { return PRG_PAGE_SIZE * prg_size; }
static size_t get_chr_size_bytes(size_t chr_size) { return CHR_PAGE_SIZE * chr_size; }

static void nes_file_free(const struct nes_file* file)
{
    free(file->prg_rom);
    free(file->chr_rom);
}

#endif //NES_EMULATOR_INES_H
