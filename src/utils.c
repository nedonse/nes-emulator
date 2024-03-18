//
// Created by quate on 3/17/2024.
//

#include "utils.h"

void set_low_byte(uint16_t* u16, uint8_t u8)
{
    *u16 = (0xFF00 & *u16) | (uint16_t) u8;
//    *(((uint8_t*) u16) + 1) = u8;  // less readable version -- relies on endianness
}


void set_high_byte(uint16_t* u16, uint8_t u8)
{
    *u16 = (0x00FF & *u16) | ((uint16_t) u8 << 8);
//    *((uint8_t*) u16) = u8;  // less readable version -- relies on endianness (also same reason union type between uint16_t and uint8_t[2] isn't used
}


uint8_t get_low_byte(uint16_t u16)
{
    return (uint8_t) u16;
}


uint8_t get_high_byte(uint16_t u16)
{
    return (uint8_t) (u16 >> 8);
}
