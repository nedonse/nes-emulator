//
// Created by quate on 2/15/2024.
//


#include <stdint.h>
#include "cpu.h"

uint8_t fetch() {
    return read(registers.pc);
}

void cpu_cycle() {
    if (curr_opcode == 0) {
        curr_opcode = read(registers.pc++);
        curr_instr_offset = 0;
    } else {
        switch (curr_opcode) {
            case LDA:
                switch (curr_instr_offset) {
                    case 0:

                }
        }
    }
}
