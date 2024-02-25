//
// Created by quate on 2/15/2024.
//

#include <stdio.h>
#include "stdlib.h"
#include "cpu.h"

#define CPU_FUNC_QUEUE_CAP 8

/// Global definitions
struct cpu_registers cpu_registers;  // TODO: explicit definition
uint8_t ir = 0;
uint16_t addr = 0;
cpu_func_ptr cpu_func_queue[CPU_FUNC_QUEUE_CAP];
size_t cpu_func_queue_start = 0;  //inclusive
size_t cpu_func_queue_end = 0;  //inclusive
size_t cpu_func_queue_size = 0;

void cpu_push_next_func(cpu_func_ptr func_ptr)
{
    if (cpu_func_queue_size == CPU_FUNC_QUEUE_CAP) {
        fprintf(stderr, "CPU function queue max capacity reached");
        exit(-1);
    }
    cpu_func_queue[cpu_func_queue_end++] = func_ptr;
    cpu_func_queue_end %= CPU_FUNC_QUEUE_CAP;
    cpu_func_queue_size++;
}

void cpu_reset()
{
    // Read reset vector and set pc to that address
    read(RST_VEC);
    cpu_registers.pc = bus_value;  // lo byte
    read(RST_VEC + 1);
    cpu_registers.pc += ((uint16_t) bus_value) << 8;  // hi byte
    // bootstrap cpu
    cpu_push_next_func(&cpu_fetch_instr);
    cpu_push_next_func(&cpu_decode);
}

void cpu_cycle()
{
    // Execute fetched instruction
    cpu_execute_instr();
}

void cpu_execute_instr()
{
    if (cpu_func_queue_size == 0) {
        fprintf(stderr, "CPU function queue empty");
        exit(-1);
    }
    cpu_func_queue[cpu_func_queue_start++]();
    cpu_func_queue_start %= CPU_FUNC_QUEUE_CAP;
    cpu_func_queue_size--;
}

void cpu_nop()
{
}

void cpu_decode()
{
    switch (ir) {
        case SEI:
            cpu_registers.sr.i = 1;  // TODO: inaccurate cycle timing; should take 2 cycles instead of 1
            break;
        case CLD:
            cpu_registers.sr.d = 0;  // TODO: see above
            break;
        case CLI:
            cpu_registers.sr.i = 0;  // TODO: see above
            break;
        case LDX_IMM:
            cpu_push_next_func(&cpu_ldx_imm);
            break;
        case TXS:
            cpu_registers.sp = cpu_registers.idx_x;  // TODO: see above
            break;
        case INX:
            cpu_registers.idx_x++;
            if (cpu_registers.idx_x & 0x80) { cpu_registers.sr.n = 1; }
            if (cpu_registers.idx_x == 0) { cpu_registers.sr.z = 0; }
            break;
        case STX_ABS:
            cpu_push_next_func(&cpu_fetch_addr_abs_lo);
            cpu_push_next_func(&cpu_fetch_addr_abs_hi);
            cpu_push_next_func(&cpu_stx);
            break;
    }
    cpu_push_next_func(&cpu_decode);
    cpu_fetch_instr();
}

void cpu_ldx_imm()
{
    cpu_registers.idx_x = ir;
    cpu_fetch_instr();
}

// Absolute addressing occurs if and only if bytes 2, 3 are 1 and 4 is 0
void cpu_fetch_addr_abs_lo()
{
    addr = (addr & 0xFF00) | (uint16_t) ir;
    cpu_fetch_instr();
}

void cpu_fetch_addr_abs_hi()
{
    addr = (addr & 0x00FF) | ((uint16_t) ir << 8);
    cpu_fetch_instr();
}

void cpu_stx()
{
    bus_value = cpu_registers.idx_x;
    write(addr);
}

void cpu_fetch_instr()
{
    read(cpu_registers.pc++);
    ir = bus_value;
}