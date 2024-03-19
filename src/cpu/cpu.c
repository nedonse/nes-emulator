//
// Created by quate on 2/15/2024.
//

#include <stdio.h>
#include <assert.h>
#include "stdlib.h"
#include "ram/ram.h"
#include "cpu.h"
#include "utils.h"


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

uint8_t pd = 0;
uint8_t ir = 0;
uint16_t addr_latch = 0;


/// CPU queue capacity
#define CPU__QUEUE_CAP 8

/// CPU queue element
struct cpu__queue_elem
{
    cpu_func_ptr cpu_mem_op;
    cpu_func_ptr cpu_exec;
    bool inc_pc;
};

/// CPU queue
struct cpu__queue_elem cpu__queue[CPU__QUEUE_CAP];
size_t cpu__queue__start = 0;  // inclusive
size_t cpu__queue__end = 0;    // inclusive
size_t cpu__queue__size = 0;


/**
 * For cycles where the CPU does not do any memory access
 */
void cpu_null_mem_op() {}

/**
 * For cycles where the CPU does not do any memory access
 */
void cpu_null_exec() {}

/**
 * Pushes a new cycle to the CPU queues.
 *
 * @param intern_ptr Internal CPU behavior for the cycle.
 * @param mem_op_ptr Memory access behavior for the cycle.
 * @param inc_pc Whether to automatically increment PC at the end of the cycle.
 */
void _cpu__push(cpu_func_ptr intern_ptr, cpu_func_ptr mem_op_ptr, bool inc_pc)
{
    if (cpu__queue__size == CPU__QUEUE_CAP) {
        fprintf(stderr, "CPU function queue max capacity reached");
        exit(ERROR_CPU_QUEUE);
    }
    cpu__queue[cpu__queue__end].cpu_mem_op = mem_op_ptr;
    cpu__queue[cpu__queue__end].cpu_exec = intern_ptr;
    cpu__queue[cpu__queue__end].inc_pc = inc_pc;
    cpu__queue__end %= CPU__QUEUE_CAP;
    cpu__queue__size++;
}

/**
 * Pushes a new cycle to the CPU queues with no memory access operation.
 *
 * @param func_ptr Internal CPU behavior.
 */
void _cpu__push_no_mem(cpu_func_ptr intern_ptr, bool inc_pc)
{
    _cpu__push(intern_ptr, &cpu_null_mem_op, inc_pc);
}

struct cpu__queue_elem _cpu__pop()
{
    if (cpu__queue__size == 0) {
        fprintf(stderr, "CPU function queue empty");
        exit(ERROR_CPU_QUEUE);
    }
    struct cpu__queue_elem next = cpu__queue[cpu__queue__start++];
    cpu__queue__start %= CPU__QUEUE_CAP;
    cpu__queue__size--;
    return next;
}

void _cpu__queue_clear()
{
    cpu__queue__start = 0;
    cpu__queue__end = 0;
    cpu__queue__size = 0;
}


/**
 * Sets PC to the reset vector address and initiates CPU
 */
void cpu_reset()
{
    // Read reset vector and set pc to that address
    addr_bus = RST_VEC_LO;
    read();
    set_low_byte(&cpu_registers.pc, data_bus);
    addr_bus = RST_VEC_HI;
    read();
    set_high_byte(&cpu_registers.pc, data_bus);

    // bootstrap cpu
    _cpu__queue_clear();
    _cpu__push(&cpu_fetch_instr, &cpu_null_exec, true);
    _cpu__push(&cpu_fetch_instr, &cpu__decode, false);  // NOTE: cpu_decode step might increment the PC based off of the opcode, so do not automatically increment the PC
}

void fetch_next_opcode() {
    addr_bus = cpu_registers.pc;
    read();
    cpu_registers.pc++;
}

uint16_t zero_page(uint8_t zp_addr) {
    return (uint16_t) zp_addr;
}

enum AddressingMode {
    IMM = 0,  // R
    ZP,       // RWM
    ZP_X,     // RWM
    ABS,      // RWM
    ABS_X,    // RWM
    ABS_Y,    // RWM
    IND_X,    // RWM
    IND_Y,    // RWM
    NUM_ADDRESSING_MODES
};

enum AddressingMode get_addressing_mode(uint8_t opcode)
{
    uint8_t addr_mode_bits = (opcode & 0b00011100) >> 2;
    switch (addr_mode_bits)
    {
        case 0: return IND_X;
        case 1: return ZP;
        case 2: return IMM;
        case 3: return ABS;
        case 4: return IND_Y;
        case 5: return ZP_X;
        case 6: return ABS_Y;
        case 7: return ABS_X;
    }
    exit(-1);
}

void read_pc() {
    addr_bus = cpu_registers.pc;
    read();
}

#define BEGIN_RESUMABLE static size_t resume_location = __LINE__; switch (resume_location) { case __LINE__:;
#define END_CYCLE resume_location = __LINE__; return; case __LINE__:;
#define END_RESUMABLE }
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
    static bool page_cross = false;
    BEGIN_RESUMABLE
    while (1) {
        ir = data_bus;

        // Load correct address into memory based on the addressing mode
        enum AddressingMode addr_mode = get_addressing_mode(ir);
        if (addr_mode == IND_X)
        {
            addr_bus = cpu_registers.pc;
            read();
            cpu_registers.pc++;

            END_CYCLE

            addr_bus = zero_page(data_bus);  // throw-away read to original address while offset is performed
            addr_latch = zero_page(data_bus + cpu_registers.idx_x);
            read();

            END_CYCLE

            addr_bus = addr_latch;
            read();

            END_CYCLE

            set_low_byte(&addr_latch, data_bus);
            addr_bus++;
            read();

            END_CYCLE

            set_high_byte(&addr_latch, data_bus);
            addr_bus = addr_latch;
            read();
        }
        else if (addr_mode == ZP)
        {
            addr_bus = cpu_registers.pc;
            read();
            cpu_registers.pc++;

            END_CYCLE

            addr_bus = zero_page(data_bus);
        }
        else if (addr_mode == IMM)
        {
            addr_bus = cpu_registers.pc;
            cpu_registers.pc++;
            read();
        }
        else if (addr_mode == ABS)
        {
            addr_bus = cpu_registers.pc;
            read();
            cpu_registers.pc++;

            END_CYCLE

            set_low_byte(&addr_latch, data_bus);
            addr_bus = cpu_registers.pc;
            read();
            cpu_registers.pc++;

            END_CYCLE

            set_high_byte(&addr_latch, data_bus);
            addr_bus = addr_latch;
            read();
        }
        else if (addr_mode == IND_Y)
        {
            addr_bus = cpu_registers.pc;
            read();
            cpu_registers.pc++;

            END_CYCLE

            addr_bus = zero_page(data_bus);
            read();

            END_CYCLE

            page_cross = data_bus + cpu_registers.idx_y < data_bus;
            set_low_byte(&addr_latch, data_bus + cpu_registers.idx_y);
            addr_bus++;
            read();

            END_CYCLE

            set_high_byte(&addr_latch, data_bus);
            addr_bus = addr_latch;
            read();

            if (page_cross)
            {
                set_high_byte(&addr_latch, get_high_byte(addr_latch) + 1);

                END_CYCLE

                addr_bus = addr_latch;
                read();
            }
        }
        else if (addr_mode == ZP_X)
        {
            addr_bus = cpu_registers.pc;
            read();
            cpu_registers.pc++;

            END_CYCLE

            addr_bus = zero_page(data_bus);  // throw-away read to original address while offset is performed
            addr_latch = zero_page(data_bus + cpu_registers.idx_x);
            read();

            END_CYCLE

            addr_bus = addr_latch;
            read();
        }
        else if (addr_mode == ABS_Y)
        {
            addr_bus = cpu_registers.pc;
            read();
            cpu_registers.pc++;

            END_CYCLE

            set_low_byte(&addr_latch, data_bus);
            addr_bus = cpu_registers.pc;
            read();
            cpu_registers.pc++;

            END_CYCLE

            set_high_byte(&addr_latch, data_bus);
            page_cross = get_low_byte(addr_latch) + cpu_registers.idx_y < get_low_byte(addr_latch);
            set_low_byte(&addr_latch, get_low_byte(addr_latch) + cpu_registers.idx_y);
            addr_bus = addr_latch;
            read();

            if (page_cross)
            {
                END_CYCLE

                set_high_byte(&addr_latch, get_high_byte(addr_latch) + 1);
                addr_bus = addr_latch;
                read();
            }
        }
        else  // Absolute, X
        {
            addr_bus = cpu_registers.pc;
            read();
            cpu_registers.pc++;

            END_CYCLE

            set_low_byte(&addr_latch, data_bus);
            addr_bus = cpu_registers.pc;
            read();
            cpu_registers.pc++;

            END_CYCLE

            set_high_byte(&addr_latch, data_bus);
            page_cross = get_low_byte(addr_latch) + cpu_registers.idx_x < get_low_byte(addr_latch);
            set_low_byte(&addr_latch, get_low_byte(addr_latch) + cpu_registers.idx_x);
            addr_bus = addr_latch;
            read();

            if (page_cross)
            {
                END_CYCLE

                set_high_byte(&addr_latch, get_high_byte(addr_latch) + 1);
                addr_bus = addr_latch;
                read();
            }
        }

        if (ir & 0b01100001 && ~ir & 0b10000010)  // ADC (011xxx01)
        {
            if (~ir & 0b00011100)  // (Indirect,X)
            {
                addr_bus = cpu_registers.pc;
                read();
                cpu_registers.pc++;

                END_CYCLE

                addr_bus = zero_page(data_bus);  // throw-away read to original address while offset is performed
                addr_latch = zero_page(data_bus + cpu_registers.idx_x);
                read();

                END_CYCLE

                addr_bus = addr_latch;
                read();

                END_CYCLE

                set_low_byte(&addr_latch, data_bus);
                addr_bus++;
                read();

                END_CYCLE

                set_high_byte(&addr_latch, data_bus);
                addr_bus = addr_latch;
                read();
            }
            else if (ir & 0b00000100 && ~ir & 0b00011000)  // Zero page
            {
                addr_bus = cpu_registers.pc;
                read();
                cpu_registers.pc++;

                END_CYCLE

                addr_bus = zero_page(data_bus);
                read();
            }
            else if (ir & 0b00001000 && ~ir & 0b00010100)  // Immediate/Implied
            {
                addr_bus = cpu_registers.pc;
                read();
                cpu_registers.pc++;
            }
            else if (ir & 0b00001100 && ~ir & 0b00010000)  // Absolute
            {
                addr_bus = cpu_registers.pc;
                read();
                cpu_registers.pc++;

                END_CYCLE

                set_low_byte(&addr_latch, data_bus);
                addr_bus = cpu_registers.pc;
                read();
                cpu_registers.pc++;

                END_CYCLE

                set_high_byte(&addr_latch, data_bus);
                addr_bus = addr_latch;
                read();
            }
            else if (ir & 0b00010000 && ~ir & 0b00001100)  // (Indirect),Y
            {
                addr_bus = cpu_registers.pc;
                read();
                cpu_registers.pc++;

                END_CYCLE

                addr_bus = zero_page(data_bus);
                read();

                END_CYCLE

                page_cross = data_bus + cpu_registers.idx_y < data_bus;
                set_low_byte(&addr_latch, data_bus + cpu_registers.idx_y);
                addr_bus++;
                read();

                END_CYCLE

                set_high_byte(&addr_latch, data_bus);
                addr_bus = addr_latch;
                read();

                if (page_cross)
                {
                    set_high_byte(&addr_latch, get_high_byte(addr_latch) + 1);

                    END_CYCLE

                    addr_bus = addr_latch;
                    read();
                }
            }
            else if (ir & 0b00010100 && ~ir & 0b00001000)  // Zero page, X
            {
                addr_bus = cpu_registers.pc;
                read();
                cpu_registers.pc++;

                END_CYCLE

                addr_bus = zero_page(data_bus);  // throw-away read to original address while offset is performed
                addr_latch = zero_page(data_bus + cpu_registers.idx_x);
                read();

                END_CYCLE

                addr_bus = addr_latch;
                read();
            }
            else if (ir & 0b00011000 && ~ir & 0b00000100)  // Absolute, Y/Implied
            {
                addr_bus = cpu_registers.pc;
                read();
                cpu_registers.pc++;

                END_CYCLE

                set_low_byte(&addr_latch, data_bus);
                addr_bus = cpu_registers.pc;
                read();
                cpu_registers.pc++;

                END_CYCLE

                set_high_byte(&addr_latch, data_bus);
                page_cross = get_low_byte(addr_latch) + cpu_registers.idx_y < get_low_byte(addr_latch);
                set_low_byte(&addr_latch, get_low_byte(addr_latch) + cpu_registers.idx_y);
                addr_bus = addr_latch;
                read();

                if (page_cross)
                {
                    END_CYCLE

                    set_high_byte(&addr_latch, get_high_byte(addr_latch) + 1);
                    addr_bus = addr_latch;
                    read();
                }
            }
            else if (ir & 0b00011100)  // Absolute, X
            {
                addr_bus = cpu_registers.pc;
                read();
                cpu_registers.pc++;

                END_CYCLE

                set_low_byte(&addr_latch, data_bus);
                addr_bus = cpu_registers.pc;
                read();
                cpu_registers.pc++;

                END_CYCLE

                set_high_byte(&addr_latch, data_bus);
                page_cross = get_low_byte(addr_latch) + cpu_registers.idx_x < get_low_byte(addr_latch);
                set_low_byte(&addr_latch, get_low_byte(addr_latch) + cpu_registers.idx_x);
                addr_bus = addr_latch;
                read();

                if (page_cross)
                {
                    END_CYCLE

                    set_high_byte(&addr_latch, get_high_byte(addr_latch) + 1);
                    addr_bus = addr_latch;
                    read();
                }
            }

            END_CYCLE

            if (data_bus + cpu_registers.acc < cpu_registers.acc) {  // TODO: this _should_ work lol
                cpu_registers.sr.v = 1;
            }
            cpu_registers.acc += data_bus;
            if (cpu_registers.acc == 0) {
                cpu_registers.sr.z = 1;
            }
            if (cpu_registers.sr.v) {
                cpu_registers.sr.c = 1;
            }
            if (cpu_registers.acc >> 7) {
                cpu_registers.sr.n = 1;
            }
        }

        else if (ir & 0b10100001 && ~ir & 0b01000010)  // LDA (101xxx01)
        {


            END_CYCLE

            if (data_bus + cpu_registers.acc < cpu_registers.acc) {  // TODO: this _should_ work lol
                cpu_registers.sr.v = 1;
            }
            cpu_registers.acc += data_bus;
            if (cpu_registers.acc == 0) {
                cpu_registers.sr.z = 1;
            }
            if (cpu_registers.sr.v) {
                cpu_registers.sr.c = 1;
            }
            if (cpu_registers.acc >> 7) {
                cpu_registers.sr.n = 1;
            }
        }
        }

        else if (ir == CLC)  // NOTE: No switch statement because of END_CYCLE
        {
            read_pc();
            END_CYCLE
            cpu_registers.sr.c = 0;
        }
        else if (ir == SEC)
        {
            read_pc();
            END_CYCLE
            cpu_registers.sr.c = 1;
        }
        else if (ir == CLI)
        {
            read_pc();
            END_CYCLE
            cpu_registers.sr.i = 0;
        }
        if (ir == SEI)
        {
            read_pc();
            END_CYCLE
            cpu_registers.sr.i = 1;
        }
        else if (ir == CLV)
        {
            read_pc();
            END_CYCLE
            cpu_registers.sr.v = 0;
        }
        else if (ir == CLD)
        {
            read_pc();
            END_CYCLE
            cpu_registers.sr.d = 0;
        }
        else if (ir == SED)
        {
            read_pc();
            END_CYCLE
            cpu_registers.sr.d = 0;
        }

        fetch_next_opcode();

        END_CYCLE
    }
    END_RESUMABLE
}