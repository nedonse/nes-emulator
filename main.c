#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "src/cpu/cpu.h"
#include "src/ram/ram.h"
#include "src/load.h"

enum CPU_State
{
    WAITING_FOR_OPCODE = 0,
    EXECUTING,
};

int main() {
    uint8_t* rom = load_rom("C:\\Users\\quate\\nes-emulator\\rom\\build\\rom.nes");
    free(rom);
    return 0;
}
