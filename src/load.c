//
// Created by quate on 2/17/2024.
//

#include "load.h"
#include "stdio.h"
#include "stdlib.h"
#include "ram/ram.h"
#include "mem.h"

#define SIZEOF16KB 0x4000
#define SIZEOF8KB 0x4000

// TODO: header validation per mapping format
struct nes_file open_file(const char* file_path)
{
    struct nes_file ret;
    FILE* file_ptr;
    uint8_t header_buffer[16];

    file_ptr = fopen(file_path, "rb");  // r for read, b for binary

    if (file_ptr == NULL) {
        fprintf(stderr, "File could not be found: %s", file_path);
        exit(EXIT_FAILURE);
    }

    fread(header_buffer, sizeof(header_buffer), 1, file_ptr);

    if (header_buffer[0] != 'N' || header_buffer[1] != 'E' || header_buffer[2] != 'S' || header_buffer[3] != 0x1A) {
        fprintf(stderr, "File has incorrect header: %s", file_path);
        exit(EXIT_FAILURE);
    }
    ret.mapper_idx = (header_buffer[6] >> 4) | (header_buffer[7] & 0xF0);
    ret.prg_size = header_buffer[4];
    ret.chr_size = header_buffer[5];
    ret.data_size = ret.prg_size * SIZEOF16KB + ret.chr_size * SIZEOF8KB;
    uint8_t* tmp = calloc(ret.data_size, sizeof(uint8_t));
    ret.data = tmp;
    fread(ret.data, sizeof(uint8_t) * ret.data_size, 1, file_ptr);

    fclose(file_ptr);

    return ret;
}

void load_file(const struct nes_file* file)
{
    switch (file->mapper_idx)
        case 0:  // NROM
            memcpy(&(cartridge[0x4000]), file->data, file->prg_size * SIZEOF16KB);
            if (file->prg_size == 1)
                memcpy(&(cartridge[0x8000]), file->data, file->prg_size * SIZEOF16KB);  // TODO: jank mirroring needs better solution
}
