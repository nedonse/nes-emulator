//
// Created by quate on 2/17/2024.
//

#include "load.h"
#include "stdio.h"
#include "stdlib.h"

#define SIZEOF16KB 0x4000
#define SIZEOF8KB 0x4000

uint8_t* load_rom(const char* file_path)
{
    FILE* file_ptr;
    uint8_t header_buffer[16];

    file_ptr = fopen(file_path, "rb");  // r for read, b for binary

    if (file_ptr == NULL) {
        fprintf(stderr, "File could not be found: %s", file_path);
        return NULL;
    }

    fread(header_buffer, sizeof(header_buffer), 1, file_ptr);

    // TODO: struct for header_buffer + memcpy?
    const size_t prg_size = header_buffer[4] * SIZEOF16KB;
    const size_t chr_size = header_buffer[5] * SIZEOF8KB;
    uint8_t* rom = calloc(prg_size + chr_size, sizeof(uint8_t));
    fread(rom, sizeof(uint8_t) * (prg_size + chr_size), 1, file_ptr);

    fclose(file_ptr);

    return rom;
}