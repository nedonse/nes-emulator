//
// Created by quate on 2/20/2024.
//

#ifndef TINY_EMULATOR_PPU_H
#define TINY_EMULATOR_PPU_H

#include <stdint.h>


struct ppu_registers
{
    struct ppu_ctrl  // NOTE: C bit-field order is compiler-defined but tend to be LSB-top; may need to replace with bit-masking
    {
        /// Nametable select (base nametable address; 0 = 0x2000, 1 = 0x2400, 2 = 0x2800, 3 = 0x2C00)
        uint8_t nt_sel : 2;
        /// Increment mode (VRAM address increment per CPU read/write of PPUDATA register; 0 = +1 = going across, 1 = +32 = going down)
        uint8_t i : 1;
        /// Sprite tile select (address of pattern table base being used for 8x8 sprites; 0 = 0x0000, 1 = 0x1000)
        uint8_t sp_sel : 1;
        /// Background tile select (address of pattern table base being used; 0 = 0x0000, 1 = 0x1000)
        uint8_t bg_sel : 1;
        /// Sprite height (0 = 8x8 pixels, 1 = 8x16 pixels)
        enum sp_h { EIGHT, SIXTEEN } sp_h : 1;
        /// PPU master/slave select (0 = read backdrop color from EXT pins, 1 = output color to EXT pins)
        uint8_t ppu_rw : 1;
        /// NMI enable (whether to generate an NMI at start of vertical blanking interval)
        uint8_t nmi : 1;
    } ppu_ctrl;

    struct ppu_mask
    {
        /// Greyscale (0 = normal, 1 = greyscale)
        uint8_t grey : 1;
        /// Background leftmost 8-pixel column enable (0 = hide, 1 = show)
        uint8_t bg_left : 1;
        /// Sprite leftmost 8-pixel column enable (0 = hide, 1 = show)
        uint8_t sp_left : 1;
        /// Background enable (0 = hide, 1 = show)
        uint8_t bg : 1;
        /// Sprite enable (0 = hide, 1 = show)
        uint8_t sp : 1;
        /// Emphasize red
        uint8_t r : 1;
        /// Emphasize green
        uint8_t g : 1;
        /// Emphasize blue
        uint8_t b : 1;
    } ppu_mask;

    struct ppu_status
    {
        /// Open bus
        uint8_t _ : 5;
        /// Sprite overflow. Represents whether more than 8 sprites are on a scanline, but is bugged: https://www.nesdev.org/wiki/PPU_sprite_evaluation
        uint8_t o : 1;
        /// Sprite 0 hit. Represents when a nonzero pixel of sprite 0 overlaps a nonzero pixel of background, cleared at dot 1 of prerender line (https://www.nesdev.org/wiki/PPU_OAM#Sprite_zero_hits)
        uint8_t s : 1;
        /// Whether vblank has started. Set at dot 1 of line 241 and cleared after read of this register or on dot 1 of prerender line
        uint8_t v : 1;
    } ppu_status;

    /// Address bus for OAM access
    uint8_t oam_addr;

    uint8_t oam_data;

    uint8_t ppu_scroll;

    /// PPU internal memory address bus
    uint8_t ppu_addr;

    /// PPU internal memory data bus
    uint8_t ppu_data;
//    uint8_t oam_dma;  // not in 0x2000-0x2007 range
};

extern struct ppu_registers ppu_registers;

extern uint8_t ppu_dot_array[242][283];

/**
 * PPU memory space
 */

/**
 * Gives a pointer to the byte corresponding to a 14-bit address in PPU address space.
 *
 * @param addr The least significant 14 bits are used for addressing and the remaining 2 bits are ignored.
 * @return Corresponding byte as a uint8.
 */
extern uint8_t* ppu_mem_map(uint16_t addr);

/**
 * Mapping function for left pattern table (0x0000-0x0FFF).
 * @param addr Only least significant 12 bits are used for addressing.
 * @return
 */
extern uint8_t* (*ppu_map_pattern_table_0)(uint16_t addr);
extern uint8_t* (*ppu_map_pattern_table_1)(uint16_t addr);
extern uint8_t* (*ppu_map_nametable_0)(uint16_t addr);
extern uint8_t* (*ppu_map_nametable_1)(uint16_t addr);
extern uint8_t* (*ppu_map_nametable_2)(uint16_t addr);
extern uint8_t* (*ppu_map_nametable_3)(uint16_t addr);
extern uint8_t* (*ppu_map_unused_space)(uint16_t addr);

extern enum mirroring { MIRRORING_H = 0, MIRRORING_V = 1 } ppu_nametable_mirroring;

#define PPU_INTERNAL_RAM_SIZE 2048
extern uint8_t ppu_ram[PPU_INTERNAL_RAM_SIZE];  // 2kB internal ppu ram

// https://www.nesdev.org/wiki/NTSC_video#Composite_decoding
// https://www.nesdev.org/wiki/PPU_palettes
#define PPU_PALETTE_RAM_SIZE 32
extern uint8_t palette_ram[PPU_INTERNAL_RAM_SIZE];

extern struct oam_entry
{
    uint8_t sprite_y;
    uint8_t sprite_tile_num;
    uint8_t sprite_attr;
    uint8_t sprite_x;
} ppu_oam[64];

void ppu_cycle();

#endif //TINY_EMULATOR_PPU_H
