#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "src/cpu/cpu.h"
#include "src/ram/ram.h"
#include "src/load.h"


int main() {
    char* rom_file = "C:\\Users\\quate\\nes-emulator\\rom\\build\\rom.nes";
    const struct nes_file nes_file = open_file(rom_file);
    load_file(&nes_file);

    cpu_reset();

    size_t ttl = 100;
    while (ttl != 0) {
        cpu_cycle();
        ttl--;
    }

    free(nes_file.data);
    return 0;
}
