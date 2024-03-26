//
// Created by quate on 2/20/2024.
//

#include "ppu.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "exit_codes.h"

#define NUM_SPRITES_PER_SCANLINE 8
#define NUM_TILES_PER_SCANLINE 32


/// Cartridge mapping functions
/**
 * Default PPU nametable mapping to internal PPU RAM.
 * @param addr
 * @return
 */
uint8_t* default_ppu_nametable(uint16_t addr)
{
    switch (ppu_nametable_mirroring)
    {
        // 0x2000 = 0x2400, 0x2800 = 0x2C00
        case MIRRORING_H: return &ppu_ram[addr & 0x03FF + (addr & 0x0800 >> 1)];  // TODO: kinda cursed
        // 0x2000 = 0x2800, 0x2400 = 0x2C00
        case MIRRORING_V: return &ppu_ram[addr & 0x07FF];
    }
}

uint8_t* (*ppu_map_pattern_table_0)(uint16_t addr) = NULL;
uint8_t* (*ppu_map_pattern_table_1)(uint16_t addr) = NULL;
uint8_t* (*ppu_map_nametable_0)(uint16_t addr) = default_ppu_nametable;
uint8_t* (*ppu_map_nametable_1)(uint16_t addr) = default_ppu_nametable;
uint8_t* (*ppu_map_nametable_2)(uint16_t addr) = default_ppu_nametable;
uint8_t* (*ppu_map_nametable_3)(uint16_t addr) = default_ppu_nametable;
uint8_t* (*ppu_map_unused_space)(uint16_t addr) = NULL;

enum mirroring ppu_nametable_mirroring;

uint8_t ppu_ram[PPU_INTERNAL_RAM_SIZE];
uint8_t ppu_palette_ram[PPU_PALETTE_RAM_SIZE];


// TODO: https://www.nesdev.org/wiki/PPU_power_up_state
// TODO: Ignore writes to registers for ~29658 CPU clock cycles
struct ppu_registers ppu_registers;  // TODO: explicit construction

/// Internal registers
// These are all the internal registers of the PPU
// https://www.nesdev.org/wiki/PPU_scrolling#PPU_internal_registers
static uint16_t ppu_v;  /// 15 bits
static uint16_t ppu_t;  /// 15 bits
static uint16_t ppu_x;  /// 3 bits
static uint16_t ppu_w;  /// 1 bit


// https://www.nesdev.org/wiki/PPU_memory_map
uint8_t* ppu_mem_map(uint16_t addr)
{
    addr &= 0x3FFF;
    switch (addr >> 12) {
        case 0b00: return ppu_map_pattern_table_0(addr);  // left
        case 0b01: return ppu_map_pattern_table_1(addr);  // right
        case 0b10:
            switch ((addr & 0x0FFF) >> 10)
            {
                case 0b00: return ppu_map_nametable_0(addr);
                case 0b01: return ppu_map_nametable_1(addr);
                case 0b10: return ppu_map_nametable_2(addr);
                case 0b11: return ppu_map_nametable_3(addr);
            }
        case 0b11:
            if ((addr & 0x0F00) != 0x0F00) return ppu_map_unused_space(addr);
            return &ppu_palette_ram[addr & 0x00FF];
        default:
            exit(ERROR_CODE__OH_NO);
    }
}


void read()
{
    ppu_registers.ppu_data = *ppu_mem_map(ppu_registers.ppu_addr);
}


void write()
{
    *ppu_mem_map(ppu_registers.ppu_addr) = ppu_registers.ppu_data;
}


#define BEGIN_RESUMABLE static size_t resume_location = __LINE__; switch (resume_location) { case __LINE__:;
#define END_CYCLE resume_location = __LINE__; return; case __LINE__:;
#define RES_CALL(call_statement) resume_location = __LINE__; case __LINE__:; if (!call_statement) { return false; }
#define END_RESUMABLE default: exit(-1); }

#define SIZE_OF_NAMETABLE_TILE 1

void ppu_cycle()
{
    BEGIN_RESUMABLE
    static size_t scanline = 0;
    // Visible scanlines
    for (; scanline < 240; ++scanline)
    {
        // TODO: cycle 0
        END_CYCLE

        static uint8_t tile = 0;
        static uint8_t curr_tile_id;  // (output of nametable)
        static uint8_t curr_tile_attr;
        static uint8_t curr_pattern_low;
        static uint8_t curr_pattern_high;

        static uint16_t nametable_base_addr;
        static uint16_t attr_base_addr;
        switch (ppu_registers.ppu_ctrl.nt_sel)
        {
            case 0: nametable_base_addr = 0x2000;
            case 1: nametable_base_addr = 0x2400;
            case 2: nametable_base_addr = 0x2800;
            case 3: nametable_base_addr = 0x2C00;
        }

        for (; tile < NUM_TILES_PER_SCANLINE; ++tile)
        {
            // TODO: sprite 0 hit
            ppu_registers.ppu_addr = nametable_base_addr + tile * SIZE_OF_NAMETABLE_TILE;
            END_CYCLE
            read();
            curr_tile_id = ppu_registers.ppu_data;
            END_CYCLE

            // TODO:
//            ppu_registers.ppu_addr = attr_base_addr + tile / 2;
            END_CYCLE
            read();
            curr_tile_attr = ppu_registers.ppu_data;
            END_CYCLE

            // TODO: read palette low bit plane
            END_CYCLE
            END_CYCLE

            // TODO: read palette high bit plane
            END_CYCLE
            // TODO: push to shift registers
            END_CYCLE
        }

        static uint8_t sprite;
        for (; sprite < NUM_SPRITES_PER_SCANLINE; ++sprite)
        {
            // TODO: sprite data fetches
            END_CYCLE
            END_CYCLE
            END_CYCLE
            END_CYCLE
            END_CYCLE
            END_CYCLE
            END_CYCLE
            END_CYCLE
        }

        // TODO: next two tiles
        END_CYCLE
        END_CYCLE
        END_CYCLE
        END_CYCLE
        END_CYCLE
        END_CYCLE
        END_CYCLE
        END_CYCLE
        END_CYCLE
        END_CYCLE
        END_CYCLE
        END_CYCLE
        END_CYCLE
        END_CYCLE
        END_CYCLE
        END_CYCLE

        // TODO: unused fetches
        END_CYCLE
        END_CYCLE
        END_CYCLE
        END_CYCLE
    }
    END_RESUMABLE
}
