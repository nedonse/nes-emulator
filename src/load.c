//
// Created by quate on 2/17/2024.
//

#include "load.h"
#include "stdio.h"
#include "stdlib.h"
#include "mem.h"
#include "exit_codes.h"
#include "cartridge/nrom00.h"

// TODO: header validation per mapping format
struct nes_file open_file(const char* file_path)
{
    struct nes_file ret;
    FILE* file_ptr;
    uint8_t header_buffer[NES_FILE_HEADER_SIZE];

    file_ptr = fopen(file_path, "rb");

    if (file_ptr == NULL)
    {
        fprintf(stderr, "File could not be found: %s", file_path);
        exit(ERROR_CODE__INVALID_FILE);
    }

    fread(header_buffer, sizeof(header_buffer), 1, file_ptr);

    if (header_buffer[0] != 'N' || header_buffer[1] != 'E' || header_buffer[2] != 'S' || header_buffer[3] != 0x1A)
    {
        fprintf(stderr, "File has incorrect header: %s", file_path);
        exit(ERROR_CODE__INVALID_FILE);
    }

    ret.mapper_idx = (header_buffer[6] >> 4) | (header_buffer[7] & 0xF0);
    ret.prg_size = header_buffer[4];
    ret.chr_size = header_buffer[5];

    ret.prg_rom = calloc(get_prg_size_bytes(ret.prg_size), sizeof(uint8_t));
    ret.chr_rom = calloc(get_chr_size_bytes(ret.chr_size), sizeof(uint8_t));
    fread(ret.prg_rom, sizeof(uint8_t), get_prg_size_bytes(ret.prg_size), file_ptr);
    fread(ret.chr_rom, sizeof(uint8_t), get_chr_size_bytes(ret.chr_size), file_ptr);

    fclose(file_ptr);

    return ret;
}

void load_file(struct nes_file* file)
{
    switch (file->mapper_idx)
    {
        case 00:  // NROM
            nrom_load(file);
            break;
        default:
            fprintf(stderr, "No implementation for iNES file load with mapper %02d", file->mapper_idx);
            exit(ERROR_CODE__UNIMPLEMENTED);
    }
}
