//
// Created by quate on 2/14/2024.
//

#ifndef TINY_EMULATOR_CPU_H
#define TINY_EMULATOR_CPU_H

#include <stdint.h>
#include "../ram/ram.h"

#define LDA 0xA9  // immed
#define LDX 0xA2  // immed

typedef struct
{
    uint8_t c : 1;
    uint8_t z : 1;
    uint8_t i : 1;
    uint8_t d : 1;
    uint8_t b : 1;
    uint8_t _ : 1;
    uint8_t v : 1;
    uint8_t n : 1;
} flags;

struct registers
{
    uint16_t pc;  // program counter
    uint8_t sp;  // stack pointer
    uint8_t acc;  // accumulator
    uint8_t idx_x;
    uint8_t idx_y;
    flags sr;  // flags
};

static struct registers registers;
static uint8_t bus_value = 0;
static uint8_t curr_opcode = 0;
static uint8_t curr_instr_offset = 0;

uint8_t fetch();
uint16_t fetch_addr();
void cpu_cycle();


#endif //TINY_EMULATOR_CPU_H