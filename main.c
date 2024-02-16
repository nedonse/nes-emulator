#include <stdio.h>
#include <stdbool.h>
#include "src/cpu/cpu.h"
#include "src/ram/ram.h"

enum CPU_State
{
    WAITING_FOR_OPCODE = 0,
    EXECUTING,
};

int main() {
    while (1) {
        uint8_t opcode = fetch();
        bool reg_a = ((opcode & 0x10) != 0);
        bool reg_x = ((opcode & 0x01) != 0);

        switch (opcode)
        {
            case LDA:
            default:
        }
    }
    return 0;
}
