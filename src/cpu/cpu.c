//
// Created by quate on 2/15/2024.
//


#include "cpu.h"

/// Global definitions
struct cpu_registers cpu_registers;
uint8_t ir = 0;
void (*curr_execute_func)() = &cpu_nop;

/// Instruction bytes
#define SEI 0x78
#define CLD 0xD8
#define CLI 0x58
#define NOP 0xEA
#define LDX_IMM 0xA2
#define TAY 0xA8
#define TSX 0xBA
#define TXA 0x8A
#define TXS 0x9A
#define TYA 0x98
#define INX 0xE8

void cpu_reset()
{
    // Read reset vector and set pc to that address
    read(RST_VEC);
    cpu_registers.pc = bus_value;  // lo byte
    read(RST_VEC + 1);
    cpu_registers.pc += ((uint16_t) bus_value) << 8;  // hi byte
}

void cpu_cycle()
{
    // Execute fetched instruction
    cpu_execute_instr();

    // Fetch
    read(cpu_registers.pc++);
    ir = bus_value;
}

void cpu_execute_instr()
{
    curr_execute_func();
}

void cpu_nop()
{
    curr_execute_func = &cpu_decode;
}

void cpu_decode()
{
    switch (ir) {
        case SEI:
            cpu_registers.sr.i = 1;
            curr_execute_func = &cpu_decode;  // TODO: inaccurate cycle timing; should take 2 cycles instead of 1
            break;
        case CLD:
            cpu_registers.sr.d = 0;
            curr_execute_func = &cpu_decode;  // TODO: see above
            break;
        case CLI:
            cpu_registers.sr.i = 0;
            curr_execute_func = &cpu_decode;  // TODO: see above
            break;
        case LDX_IMM:
            curr_execute_func = &cpu_ldx_imm;
            break;
        case TXS:
            cpu_registers.sp = cpu_registers.idx_x;  // TODO: see above
        case INX:
            cpu_registers.idx_x++;
            if (cpu_registers.idx_x & 0x80) { cpu_registers.sr.n = 1; }
            if (cpu_registers.idx_x == 0) { cpu_registers.sr.z = 0; }
    }
}

void cpu_ldx_imm()
{
    cpu_registers.idx_x = ir;
    curr_execute_func = &cpu_decode;
}
