//
// Created by quate on 2/14/2024.
//

#ifndef NES_EMULATOR__CPU_H
#define NES_EMULATOR__CPU_H

#include <stdint.h>
// TODO: sort out private and public variables, organize

/// Instruction bytes
/// ==================== INTERNAL/IMPLIED Instructions ========================

#define CLC 0x18
#define SEC 0x38
#define CLI 0x58
#define SEI 0x78
#define CLV 0xB8
#define CLD 0xD8
#define SED 0xF8

#define TAY 0xA8
#define TXA 0x8A
#define TAX 0xAA
#define TYA 0x98
#define TXS 0x9A
#define TSX 0xBA

#define DEY 0x88
#define INY 0xC8
#define INX 0xE8
#define DEX 0xCA

#define NOP 0xEA

/// ===================== READ Instructions ======================

#define NMI_VEC_LO 0xFFFA
#define NMI_VEC_HI 0xFFFB
static const uint16_t RST_VEC_LO = 0xFFFC;
static const uint16_t RST_VEC_HI = 0xFFFD;
#define IRQ_VEC_LO 0xFFFE
#define IRQ_VEC_HI 0xFFFF

#define STACK_PAGE_START 0x0100

/**
 * CPU status register flags
 */
typedef union
{
    struct __attribute__((__packed__))  // TODO: platform dependent lol
    {
        uint8_t c : 1;
        uint8_t z : 1;
        uint8_t i : 1;
        uint8_t d : 1;
        uint8_t b : 1;
        uint8_t _ : 1;
        uint8_t v : 1;
        uint8_t n : 1;
    };
    uint8_t u8;
} flags;

/**
 * CPU registers
 */
struct cpu_registers
{
    uint16_t pc;    /// program counter
    uint8_t sp;     /// stack pointer
    uint8_t acc;    /// accumulator
    uint8_t idx_x;  /// index x
    uint8_t idx_y;  /// index y
    flags sr;       /// flags
};

extern struct cpu_registers cpu_registers;

/// Instruction register
extern uint8_t cpu_ir;

/**
 * Address latch used for parsing and storing an address in memory specified by instructions
 */
extern uint16_t cpu_addr_latch;


uint8_t* cpu_mem_map(uint16_t addr);
extern uint8_t* (*cpu_cartridge_space_map)(uint16_t addr);

/// Sends read signal to memory bus
void cpu_read();

/// Sends write signal to memory bus
void cpu_write();

/** =================================================== */

void cpu_reset();
void cpu_cycle();

#endif //NES_EMULATOR__CPU_H
