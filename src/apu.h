//
// Created by quate on 3/25/2024.
//
// Contains APU and a few other I/O registers used by the NES.
//

#ifndef NES_EMULATOR_APU_H
#define NES_EMULATOR_APU_H

#include <stdint.h>
#include <stdbool.h>

#define APU_IO_REGISTER_SIZE 18

// https://www.nesdev.org/wiki/2A03
// TODO: again, sus bit fields and reliance on specific struct memory layout
struct apu_pulse
{
    uint8_t duty : 2;
    bool envelope_loop : 1;
    bool const_vol : 1;
    uint8_t volume : 4;
    bool sweep_enabled : 1;
    uint8_t sweep_period : 3;
    bool sweep_negate : 1;
    uint8_t sweep_shift : 3;
    uint8_t timer_low;
    uint8_t length_counter_load : 5;
    uint8_t timer_high : 3;
};

extern union apu_registers
{
    struct apu_registers_str
    {
        struct apu_pulse pulse_1;
        struct apu_pulse pulse_2;
        struct apu_triangle {} triangle;
        struct apu_noise {} noise;
        struct apu_dmc {} dmc;
        struct apu_status {} status;
        struct apu_frame_counter {} frame_counter;
    } registers;
    uint8_t array[APU_IO_REGISTER_SIZE];
} apu_registers;

#endif //NES_EMULATOR_APU_H
