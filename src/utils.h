//
// Created by quate on 3/17/2024.
//

#include <stdint.h>

#ifndef NES_EMULATOR_UTILS_H
#define NES_EMULATOR_UTILS_H

/**
 * Endian-independent
 * @param u16
 * @param u8
 */
void set_low_byte(uint16_t* u16, uint8_t u8);
void set_high_byte(uint16_t* u16, uint8_t u8);
uint8_t get_low_byte(uint16_t u16);
uint8_t get_high_byte(uint16_t u16);

#endif //NES_EMULATOR_UTILS_H
