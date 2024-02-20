//
// Created by quate on 2/15/2024.
//


#include "cpu.h"

struct registers registers;
uint8_t ir = 0;
uint8_t curr_instr_offset = 0;

void cpu_reset() {
    read(RST_VEC);
    registers.pc = bus_value;  // lo byte
    read(RST_VEC + 1);
    registers.pc += ((uint16_t) bus_value) << 8;  // hi byte
}

void cpu_cycle() {
    read(registers.pc++);
    ir = bus_value;
}
