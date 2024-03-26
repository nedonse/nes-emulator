#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "cpu/cpu.h"
#include "ppu.h"
#include "load.h"


const size_t ppu_cycles_per_cpu_cycle = 3;


int main() {
    const char* rom_file = "C:\\Users\\quate\\nes-emulator\\rom\\build\\rom.nes";
    struct nes_file nes_file = open_file(rom_file);
    load_file(&nes_file);

    cpu_reset();

    size_t ttl = 500;
    size_t ppu_counter = 0;
    while (ttl != 0)
    {
        cpu_cycle();

        if (++ppu_counter == ppu_cycles_per_cpu_cycle)
        {
            ppu_cycle();
            ppu_counter = 0;
        }

        ttl--;
    }

    nes_file_free(&nes_file);
    return 0;
}
