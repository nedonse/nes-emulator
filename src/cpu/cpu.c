//
// Created by quate on 2/15/2024.
//

#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include "stdlib.h"
#include "cpu.h"
#include "utils.h"
#include "exit_codes.h"
#include "ppu.h"
#include "apu.h"


// TODO: https://www.nesdev.org/wiki/CPU_power_up_state
struct cpu_registers cpu_registers = {
    .pc = 0,
    .sp = 0,
    .acc = 0,
    .idx_x = 0,
    .idx_y = 0,
    .sr = {
        .c = 0,
        .z = 0,
        .i = 0,
        .d = 0,
        .b = 0,
        .v = 0,
        .n = 0
    }
};


uint8_t cpu_ir = 0;
uint16_t cpu_addr_latch = 0;


#define INTERNAL_RAM_UPPER 0x2000
#define RAM_MASK 0x7FF
#define PPU_REG_SPACE_UPPER 0x4000
#define PPU_REG_MASK 0x7
#define APU_IO_REG_SPACE_UPPER 0x4018
#define APU_IO_REG_MASK 0xFF
#define RAM_SIZE 0x0800  // 2kB

uint8_t ram[RAM_SIZE];
uint16_t addr_bus = 0;
uint8_t data_bus = 0;

/**
 * Returns a pointer to the byte located at the provided memory address.
 *
 * The NES memory map has the following layout:
 * 0x0000-0x07FF: Internal RAM
 * 0x0800-0x1FFF: Mirrors of 0x0000-0x07FF
 * 0x2000-0x2007: CPU view of PPU registers
 * 0x2008-0x3FFF: Mirrors of 0x2000-0x2007
 * 0x4000-0xFFFF: Device and cartridge space (exact layout and memory usage depends on the cartridge)
 *
 * @param addr The 16-bit address to look up.
 * @return A pointer to the byte. Returns NULL if the address is invalid.
 */
uint8_t* cpu_mem_map(uint16_t addr)
{
    if (addr < INTERNAL_RAM_UPPER)
    {
        return &ram[addr & RAM_MASK];
    }
    else if (addr < PPU_REG_SPACE_UPPER)
    {
        return &((uint8_t*) &ppu_registers)[addr & PPU_REG_MASK];  // TODO: see other register todos
    }
    else if (addr < APU_IO_REG_SPACE_UPPER)
    {
        return &apu_registers.array[addr & APU_IO_REG_MASK];
    }
    else
    {
        return cpu_cartridge_space_map(addr);
    }
}


uint8_t* (*cpu_cartridge_space_map)(uint16_t addr) = NULL;


void cpu_read()
{
    if (cpu_mem_map(addr_bus) != NULL) {
        data_bus = *cpu_mem_map(addr_bus);
    }
    // Default open bus
}

void cpu_write()
{
    if (cpu_mem_map(addr_bus) != NULL) {
        *cpu_mem_map(addr_bus) = data_bus;
    }
    // Default open bus
}


/**
 * Sets PC to the reset vector address and initiates CPU
 */
void cpu_reset()
{
    // Read reset vector and set pc to that address
    addr_bus = RST_VEC_LO;
    cpu_read();
    set_low_byte(&cpu_registers.pc, data_bus);
    addr_bus = RST_VEC_HI;
    cpu_read();
    set_high_byte(&cpu_registers.pc, data_bus);
}


uint16_t zero_page(uint8_t zp_addr) {
    return (uint16_t) zp_addr;
}

enum AddressingMode {
    IMM = 0,  // R
    ZP,       // RWM
    ZP_X,     // RWM
    ZP_Y,     // R
    ABS,      // RWM
    ABS_X,    // RWM
    ABS_Y,    // RW
    IND_X,    // RW
    IND_Y,    // RW
    ACC,      //   M
    NUM_ADDRESSING_MODES
};

/**
 * All non-implied and non-relative address modes
 * @param opcode
 * @return
 */
enum AddressingMode get_addressing_mode(uint8_t opcode)
{
    uint8_t addr_mode_bits = (opcode & 0b00011100) >> 2;
    switch (addr_mode_bits)
    {
        case 0: return opcode & 1 ? IND_X : IMM;
        case 1: return ZP;
        case 2: return opcode & 1 ? IMM : ACC;
        case 3: return ABS;
        case 4: return IND_Y;
        case 5: return ZP_X;
        case 6: return ABS_Y;
        case 7: return ABS_X;
    }
    exit(ERROR_CODE__OH_NO);
}

enum ReadWrite {
    NONE = 0,
    READ,
    WRITE,
    MODIFY,
};

enum Instruction
{
    LDX,
    LDA,
    LDY,
    STX,
    STA,
    STY,
    ADC,
    BIT,
    BPL,
    BMI,
    BVC,
    BVS,
    BCC,
    BCS,
    BNE,
    BEQ,
    NUM_INSTRUCTIONS
};


enum Instruction get_instr(const uint8_t opcode)
{
    if (!((opcode ^ 0b10100010) & 0b11100011)) { return LDX; }
    if (!((opcode ^ 0b10100001) & 0b11100011)) { return LDA; }
    if (!((opcode ^ 0b10000010) & 0b11100011)) { return STX; }
    if (!((opcode ^ 0b01100001) & 0b11100011)) { return ADC; }
    if (!((opcode ^ 0b00100100) & 0b11110111)) { return BIT; }
    if (!((opcode ^ 0b00010000) & 0b00011111))  // Branching
    {
        switch (opcode >> 5)
        {
            case 0: return BPL;
            case 1: return BMI;
            case 2: return BVC;
            case 3: return BVS;
            case 4: return BCC;
            case 5: return BCS;
            case 6: return BNE;
            case 7: return BEQ;
        }
    }
    fprintf(stderr, "Unimplemented opcode encountered");
    exit(ERROR_CODE__UNIMPLEMENTED);
}

bool is_branch_instr(const uint8_t opcode)
{
    return !((opcode ^ 0b00010000) & 0b00011111);
}

void read_pc() {
    addr_bus = cpu_registers.pc;
    cpu_read();
}

#define BEGIN_RESUMABLE static size_t resume_location = __LINE__; switch (resume_location) { case __LINE__:;
#define END_CYCLE resume_location = __LINE__; return; case __LINE__:;
#define RES_CALL(call_statement) resume_location = __LINE__; case __LINE__:; if (!call_statement) { return false; }
#define END_RESUMABLE default: exit(-1); }
/**
 * Runs a single cycle of the CPU.
 *
 * Each cycle, the CPU is capable of executing internal logic and a memory bus read/write operation.
 *
 * CPU internal operations occur before memory access operations.
 *
 * Pops the CPU function queue.
 */
void cpu_cycle()
{
    BEGIN_RESUMABLE
    while (1) {
        read_pc();
        cpu_registers.pc++;

        END_CYCLE

        cpu_ir = data_bus;

        // ================ Implied ================= //
        // NOTE: No switch statement because of END_CYCLE
        if (cpu_ir == CLC)
        {
            read_pc();
            END_CYCLE
            cpu_registers.sr.c = 0;
            continue;
        }
        else if (cpu_ir == SEC)
        {
            read_pc();
            END_CYCLE
            cpu_registers.sr.c = 1;
            continue;
        }
        else if (cpu_ir == CLI)
        {
            read_pc();
            END_CYCLE
            cpu_registers.sr.i = 0;
            continue;
        }
        else if (cpu_ir == SEI)
        {
            read_pc();
            END_CYCLE
            cpu_registers.sr.i = 1;
            continue;
        }
        else if (cpu_ir == CLV)
        {
            read_pc();
            END_CYCLE
            cpu_registers.sr.v = 0;
            continue;
        }
        else if (cpu_ir == CLD)
        {
            read_pc();
            END_CYCLE
            cpu_registers.sr.d = 0;
            continue;
        }
        else if (cpu_ir == SED)
        {
            read_pc();
            END_CYCLE
            cpu_registers.sr.d = 0;
            continue;
        }
        else if (cpu_ir == TAY)
        {
            read_pc();
            cpu_registers.idx_y = cpu_registers.acc;
            cpu_registers.sr.z = cpu_registers.idx_y == 0;
            cpu_registers.sr.n = cpu_registers.idx_y >> 7;
            continue;
        }  // NOTE: I'm not sure if flags are for the result of the copy or the value before the copy
        else if (cpu_ir == TXA)
        {
            read_pc();
            END_CYCLE
            cpu_registers.acc = cpu_registers.idx_x;
            cpu_registers.sr.z = cpu_registers.acc == 0;
            cpu_registers.sr.n = cpu_registers.acc >> 7;
            continue;
        }
        else if (cpu_ir == TAX)
        {
            read_pc();
            END_CYCLE
            cpu_registers.idx_x = cpu_registers.acc;
            cpu_registers.sr.z = cpu_registers.idx_x == 0;
            cpu_registers.sr.n = cpu_registers.idx_x >> 7;
            continue;
        }
        else if (cpu_ir == TYA)
        {
            read_pc();
            END_CYCLE
            cpu_registers.acc = cpu_registers.idx_y;
            cpu_registers.sr.z = cpu_registers.acc == 0;
            cpu_registers.sr.n = cpu_registers.acc >> 7;
            continue;
        }
        else if (cpu_ir == TXS)
        {
            read_pc();
            END_CYCLE
            cpu_registers.sp = cpu_registers.idx_x;
            continue;
        }
        else if (cpu_ir == TSX)
        {
            read_pc();
            END_CYCLE
            cpu_registers.idx_x = cpu_registers.sp;
            cpu_registers.sr.z = cpu_registers.idx_x == 0;
            cpu_registers.sr.n = cpu_registers.idx_x >> 7;
            continue;
        }
        else if (cpu_ir == DEY)
        {
            read_pc();
            END_CYCLE
            cpu_registers.idx_y--;
            cpu_registers.sr.z = cpu_registers.idx_y == 0;
            cpu_registers.sr.n = cpu_registers.idx_y >> 7;
            continue;
        }
        else if (cpu_ir == INY)
        {
            read_pc();
            END_CYCLE
            cpu_registers.idx_y++;
            cpu_registers.sr.z = cpu_registers.idx_y == 0;
            cpu_registers.sr.n = cpu_registers.idx_y >> 7;
            continue;
        }
        else if (cpu_ir == INX)
        {
            read_pc();
            END_CYCLE
            cpu_registers.idx_x++;
            cpu_registers.sr.z = cpu_registers.idx_x == 0;
            cpu_registers.sr.n = cpu_registers.idx_x >> 7;
            continue;
        }
        else if (cpu_ir == DEX)
        {
            read_pc();
            END_CYCLE
            cpu_registers.idx_x--;
            cpu_registers.sr.z = cpu_registers.idx_x == 0;
            cpu_registers.sr.n = cpu_registers.idx_x >> 7;
            continue;
        }


        enum AddressingMode addr_mode;
        static enum Instruction instr;
        static enum ReadWrite rw = NONE;

        static bool page_cross = false;

        addr_mode = get_addressing_mode(cpu_ir);
        instr = get_instr(cpu_ir);

        // If branch instr, addressing mode is a branch-only mode called "relative"; forego usual control flow
        if (is_branch_instr(cpu_ir))
        {
            // Fetch operand
            read_pc();
            cpu_registers.pc++;

            END_CYCLE

            if (instr == BPL && !cpu_registers.sr.n ||
                instr == BMI && cpu_registers.sr.n ||
                instr == BVC && !cpu_registers.sr.v ||
                instr == BVS && cpu_registers.sr.v ||
                instr == BCC && !cpu_registers.sr.c ||
                instr == BCS && cpu_registers.sr.c ||
                instr == BNE && !cpu_registers.sr.z ||
                instr == BEQ && cpu_registers.sr.z)
            {
                int8_t offset = (int8_t) data_bus;
                read_pc();  // dummy read
                static uint16_t true_addr;
                true_addr = cpu_registers.pc + offset;
                set_low_byte(&cpu_registers.pc, get_low_byte(true_addr));

                END_CYCLE

                if (cpu_registers.pc != true_addr)
                {
                    read_pc();  // dummy read
                    set_high_byte(&cpu_registers.pc, get_high_byte(true_addr));

                    END_CYCLE
                }
            }

            continue;
        }

        rw = NONE;
        if (instr == LDX)
        {
            rw = READ;
            if (addr_mode == ZP_X) { addr_mode = ZP_Y; }
            if (addr_mode == ABS_X) { addr_mode = ABS_Y; }
        }
        if (instr == STX)
        {
            rw = WRITE;
            if (addr_mode == ZP_X) { addr_mode = ZP_Y; }
            if (addr_mode == ABS_X || addr_mode == ACC || addr_mode == IND_Y || addr_mode == ABS_Y) {
                exit(ERROR_CODE__UNIMPLEMENTED);
            }
        }
        if (instr == BIT)
        {
            rw = READ;
            if (addr_mode != ZP_X && addr_mode != ABS) { exit(ERROR_CODE__OH_NO); }
        }

        page_cross = false;

        if (addr_mode == IMM)
        {
            addr_bus = cpu_registers.pc;
            cpu_registers.pc++;
        }
        else if (addr_mode == ZP)
        {
            read_pc();
            cpu_registers.pc++;

            END_CYCLE

            addr_bus = zero_page(data_bus);
        }
        else if (addr_mode == ABS)
        {
            read_pc();
            cpu_registers.pc++;

            END_CYCLE

            set_low_byte(&cpu_addr_latch, data_bus);
            read_pc();
            cpu_registers.pc++;

            END_CYCLE

            set_high_byte(&cpu_addr_latch, data_bus);
            addr_bus = cpu_addr_latch;
        }
        else if (addr_mode == ZP_X)
        {
            read_pc();
            cpu_registers.pc++;

            END_CYCLE

            cpu_addr_latch = zero_page(data_bus + cpu_registers.idx_x);
            addr_bus = zero_page(data_bus);  // throw-away read to original address while offset is performed
            cpu_read();

            END_CYCLE

            addr_bus = cpu_addr_latch;
        }
        else if (addr_mode == ZP_Y)
        {
            read_pc();
            cpu_registers.pc++;

            END_CYCLE

            cpu_addr_latch = zero_page(data_bus + cpu_registers.idx_y);
            addr_bus = zero_page(data_bus);  // throw-away read to original address while offset is performed
            cpu_read();

            END_CYCLE

            addr_bus = cpu_addr_latch;
        }
        else if (addr_mode == ABS_X)
        {
            read_pc();
            cpu_registers.pc++;

            END_CYCLE

            set_low_byte(&cpu_addr_latch, data_bus);
            read_pc();
            cpu_registers.pc++;

            END_CYCLE

            set_high_byte(&cpu_addr_latch, data_bus);
            page_cross = get_low_byte(cpu_addr_latch) + cpu_registers.idx_x < get_low_byte(cpu_addr_latch);
            set_low_byte(&cpu_addr_latch, get_low_byte(cpu_addr_latch) + cpu_registers.idx_x);
            addr_bus = cpu_addr_latch;

            if (page_cross)
            {
                cpu_read();

                END_CYCLE

                set_high_byte(&cpu_addr_latch, get_high_byte(cpu_addr_latch) + 1);
                addr_bus = cpu_addr_latch;
            }
        }
        else if (addr_mode == ABS_Y)
        {
            read_pc();
            cpu_registers.pc++;

            END_CYCLE

            set_low_byte(&cpu_addr_latch, data_bus);
            read_pc();
            cpu_registers.pc++;

            END_CYCLE

            set_high_byte(&cpu_addr_latch, data_bus);
            page_cross = get_low_byte(cpu_addr_latch) + cpu_registers.idx_y < get_low_byte(cpu_addr_latch);
            set_low_byte(&cpu_addr_latch, get_low_byte(cpu_addr_latch) + cpu_registers.idx_y);
            addr_bus = cpu_addr_latch;

            if (page_cross)
            {
                cpu_read();

                END_CYCLE

                set_high_byte(&cpu_addr_latch, get_high_byte(cpu_addr_latch) + 1);
                addr_bus = cpu_addr_latch;
            }
        }
        else if (addr_mode == IND_X)
        {
            read_pc();
            cpu_registers.pc++;

            END_CYCLE

            cpu_addr_latch = zero_page(data_bus + cpu_registers.idx_x);
            addr_bus = zero_page(data_bus);  // throw-away read to original address while offset is performed
            cpu_read();

            END_CYCLE

            addr_bus = cpu_addr_latch;
            cpu_read();

            END_CYCLE

            set_low_byte(&cpu_addr_latch, data_bus);
            addr_bus++;
            cpu_read();

            END_CYCLE

            set_high_byte(&cpu_addr_latch, data_bus);
            addr_bus = cpu_addr_latch;
        }
        else if (addr_mode == IND_Y)
        {
            read_pc();
            cpu_registers.pc++;

            END_CYCLE

            addr_bus = zero_page(data_bus);
            cpu_read();

            END_CYCLE

            page_cross = data_bus + cpu_registers.idx_y < data_bus;
            set_low_byte(&cpu_addr_latch, data_bus + cpu_registers.idx_y);
            addr_bus++;
            cpu_read();

            END_CYCLE

            set_high_byte(&cpu_addr_latch, data_bus);
            addr_bus = cpu_addr_latch;

            if (page_cross)
            {
                cpu_read();
                set_high_byte(&cpu_addr_latch, get_high_byte(cpu_addr_latch) + 1);

                END_CYCLE

                addr_bus = cpu_addr_latch;
            }
        }
        else  // ACC
        {
            // FIXME: erm
        }

        if (rw == READ)
        {
            cpu_read();
        }

        // Load write instruction values into data_bus
        if (instr == STX)
        {
            data_bus = cpu_registers.idx_x;
        }

        if (rw == WRITE)
        {
            cpu_write();
        }

        END_CYCLE

        // Read instruction last-cycle behaviors
        if (instr == ADC)
        {
            cpu_registers.sr.v = (cpu_registers.acc + data_bus + cpu_registers.sr.c < cpu_registers.acc);
            cpu_registers.acc += data_bus + cpu_registers.sr.c;
            cpu_registers.sr.z = cpu_registers.acc == 0;
            cpu_registers.sr.c = cpu_registers.sr.v;
            cpu_registers.sr.n = cpu_registers.acc >> 7;
        }

        else if (instr == LDA)
        {
            cpu_registers.acc = data_bus;
        }

        else if (instr == LDX)
        {
            cpu_registers.idx_x = data_bus;
        }

        else if (instr == BIT)
        {
            cpu_registers.sr.z = ((cpu_registers.acc & data_bus) == 0);
            cpu_registers.sr.v = (data_bus & 0x40) >> 6;
            cpu_registers.sr.n = data_bus >> 7;
        }
    }
    END_RESUMABLE
}