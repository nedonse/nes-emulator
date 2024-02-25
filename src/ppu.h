//
// Created by quate on 2/20/2024.
//

#ifndef TINY_EMULATOR_PPU_H
#define TINY_EMULATOR_PPU_H

#include <stdint.h>

// NOTE: bit field size is compiler-defined, so it may be necessary
// to replace this with a uint8_t with functions/constants to access
// flags
struct ppu_ctrl
{
    uint8_t nametable_sel : 2;
    uint8_t inc_mode : 1;
    uint8_t sprite_sel : 1;
    uint8_t bg_sel : 1;
    uint8_t sprite_height : 1;
    uint8_t ppu_master_sel : 1;
    uint8_t nmi : 1;
};

struct ppu_registers
{
    struct ppu_ctrl ppu_ctrl;
    uint8_t ppu_mask;  // TODO: most of these registers are placeholders for bit fields?
    uint8_t ppu_status;
    uint8_t oam_addr;
    uint8_t oam_data;
    uint8_t ppu_scroll;
    uint8_t ppu_addr;
    uint8_t ppu_data;
//    uint8_t oam_dma;  // not in 0x2000-0x2007 range
};

extern struct ppu_registers ppu_registers;

#endif //TINY_EMULATOR_PPU_H
