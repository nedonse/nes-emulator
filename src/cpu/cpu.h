//
// Created by quate on 2/14/2024.
//

#ifndef TINY_EMULATOR_CPU_H
#define TINY_EMULATOR_CPU_H

#include <stdint.h>
#include "../ram/ram.h"

/// Instruction bytes
#define SEI 0x78
#define CLD 0xD8
#define CLI 0x58
#define NOP 0xEA
#define LDX_IMM 0xA2
#define STX_ABS 0x8E
#define TAY 0xA8
#define TSX 0xBA
#define TXA 0x8A
#define TXS 0x9A
#define TYA 0x98
#define INX 0xE8

#define NMI_VEC 0xFFFA
static const uint16_t RST_VEC = 0xFFFC;
#define IRQ_VEC 0xFFFE

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

struct cpu_registers
{
    uint16_t pc;  // program counter
    uint8_t sp;  // stack pointer
    uint8_t acc;  // accumulator
    uint8_t idx_x;
    uint8_t idx_y;
    flags sr;  // flags
};

extern struct cpu_registers cpu_registers;
extern uint8_t ir;
extern uint16_t addr;  /// used for resolving memory addresses in instructions

typedef void (*cpu_func_ptr)();

void cpu_reset();
void cpu_cycle();
void cpu_execute_instr();

void cpu_nop();
void cpu_decode();
void cpu_ldx_imm();
void cpu_fetch_addr_abs_lo();
void cpu_fetch_addr_abs_hi();
void cpu_stx();

void cpu_fetch_instr();

#endif //TINY_EMULATOR_CPU_H
