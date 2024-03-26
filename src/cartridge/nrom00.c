//
// Created by quate on 3/25/2024.
//

#include "nrom00.h"
#include "cpu/cpu.h"
#include "ppu.h"
#include "exit_codes.h"


#define PRG_ROM_ADDR_LOWER 0x8000


struct nes_file* nes_file = NULL;


uint8_t* nrom_pattern_table_0(uint16_t addr)
{
    return &nes_file->chr_rom[addr & 0x1FFF];
}

uint8_t* nrom_pattern_table_1(uint16_t addr)
{
    return &nes_file->chr_rom[addr & 0x1FFF];
}

uint8_t* nrom128_cpu_cartridge_space_map(uint16_t addr)
{
    if (addr < PRG_ROM_ADDR_LOWER)
        return NULL;
    else
        return &nes_file->prg_rom[addr & 0x3FFF];  // Mirroring between 0x8000-0xBFFF and 0xC000-0xFFFF
}

uint8_t* nrom256_cpu_cartridge_space_map(uint16_t addr)
{
    if (addr < PRG_ROM_ADDR_LOWER)
        return NULL;
    else
        return &nes_file->prg_rom[addr & 0x7FFF];
}

void nrom_load(struct nes_file* file)
{
    ppu_map_pattern_table_0 = &nrom_pattern_table_0;
    ppu_map_pattern_table_1 = &nrom_pattern_table_1;
    if (file->prg_size == 1)
        cpu_cartridge_space_map = &nrom128_cpu_cartridge_space_map;
    else if (file->prg_size == 2)
        cpu_cartridge_space_map = &nrom256_cpu_cartridge_space_map;
    else
        exit(ERROR_CODE__INVALID_FILE);
    ppu_nametable_mirroring = MIRRORING_H;
    nes_file = file;
}
