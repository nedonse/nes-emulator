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
    struct cpu__queue_elem next = _cpu__pop();
    next.cpu_exec();
    next.cpu_mem_op();
    if (next.inc_pc) {
        cpu_registers.pc++;
    }
}

void cpu_fetch_instr()
{
    addr_bus = cpu_registers.pc;
    read();
    pd = data_bus;
}

void cpu_new_opcode()
{

}


/**
 * Sets low byte of address latch to byte at PC address
 */
void cpu_read_addr_lo()
{
    addr_bus = cpu_registers.pc;
    read();
    set_low_byte(&addr_latch, data_bus);
}


/**
 * Sets high byte of address latch to byte at PC address
 */
void cpu_read_addr_hi()
{
    addr_bus = cpu_registers.pc;
    read();
    set_high_byte(&addr_latch, data_bus);
}


/**
 * Clears high byte of address latch for zero page addressing operations
 */
void cpu_clear_addr_hi()
{
    addr_latch = addr_latch & 0x00FF;
}


/**
 * Triggers read operation on address stored in address latch
 */
void cpu_read_at_addr()
{
    addr_bus = addr_latch;
    read();
}


/**
 * Loads last read byte into accumulator
 */
void cpu_load_into_a()
{
    cpu_registers.acc = data_bus;
}


/**
 * Loads last read byte into x register
 */
void cpu_load_into_x()
{
    cpu_registers.idx_x = data_bus;
}


/**
 * Loads last read byte into y register
 */
void cpu_load_into_y()
{
    cpu_registers.idx_y = data_bus;
}


/**
 * Stores accumulator value into memory at address latch
 */
void cpu_store_from_a()
{
    data_bus = cpu_registers.acc;
    addr_bus = addr_latch;
    write();
}


/**
 * Stores x register into memory at address latch
 */
void cpu_store_from_x()
{
    data_bus = cpu_registers.idx_x;
    addr_bus = addr_latch;
    write();
}


/**
 * Stores y register into memory at address latch
 */
void cpu_store_from_y()
{
    data_bus = cpu_registers.idx_y;
    addr_bus = addr_latch;
    write();
}


/**
 * Adds value in data bus to accumulator
 *
 * Sets carry and overflow flags if overflow occurs, zero flag if resulting accumulator value is 0, and negative flag if bit 7 is set.
 */
void cpu_add_to_acc()
{
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


/**
 * Bit-wise AND on accumulator with value in data bus.
 *
 * Sets zero flag if resulting acc value is 0 and negative flag if resulting acc value bit 7 is set.
 */
void cpu_and_with_acc()
{
    cpu_registers.acc = cpu_registers.acc & data_bus;
    if (cpu_registers.acc == 0) {
        cpu_registers.sr.z = 1;
    }
    if (cpu_registers.acc >> 7) {
        cpu_registers.sr.n = 1;
    }
}


// TODO:
void cpu_arith_shift_left()
{
}


// TODO:
void cpu_arith_shift_left_acc()
{
}


#define CPU__COND_BRANCH(check_expr) \
    uint8_t addr_offset = data_bus; \
    addr_bus = cpu_registers.pc;    \
    read();                         \
    if (check_expr) {               \
        *((uint8_t*) &cpu_registers.pc) += addr_offset; /* TODO: Push cpu_fix_pch */ \
    } else { \
        cpu_registers.pc++; \
        cpu_new_opcode(); \
    }


void cpu_branch_if_z_set()
{
    CPU__COND_BRANCH(cpu_registers.sr.z)
}


void cpu_branch_if_c_set()
{
    CPU__COND_BRANCH(cpu_registers.sr.c)
}


void cpu_branch_if_n_set()
{
    CPU__COND_BRANCH(cpu_registers.sr.n)
}


void cpu_branch_if_v_set()
{
    CPU__COND_BRANCH(cpu_registers.sr.v)
}


void cpu_branch_if_c_clear()
{
    CPU__COND_BRANCH(cpu_registers.sr.c == 0)
}


void cpu_branch_if_z_clear()
{
    CPU__COND_BRANCH(cpu_registers.sr.z == 0)
}


void cpu_branch_if_n_clear()
{
    CPU__COND_BRANCH(cpu_registers.sr.n == 0)
}


void cpu_branch_if_v_clear()
{
    CPU__COND_BRANCH(cpu_registers.sr.v == 0)
}


/**
 * Takes bit-wise AND between acc and data bus value and throws away the result. Sets flags accordingly.
 *
 * Zero flag set if result is 0.
 * Overflow flag set if bit 6 of result is set.
 * Negative flag set if bit 7 of result is set.
 */
void cpu_bit_test()
{
    uint8_t res = cpu_registers.acc & data_bus;
    cpu_registers.sr.z = (res == 0);
    cpu_registers.sr.v = (res & 0x4000);
    cpu_registers.sr.n = (res & 0x8000);
    // END of BIT
}


/**
 * ============================= Stack operations ===============================
 */

/**
 *
 * @param sp
 * @return
 */
uint16_t sp_to_address(uint8_t sp)
{
    return STACK_PAGE_START + sp;
}


void stack_push(uint8_t val)
{
    addr_bus = sp_to_address(cpu_registers.sp);
    cpu_registers.sp--;
    data_bus = val;
    write();
}


/**
 * ======================== BRK cycles ==============================
 */


void cpu_push_pc_high_set_b_flag()
{
    cpu_registers.sr.b = 1;
    stack_push(get_high_byte(cpu_registers.pc));
}


void cpu_push_pc_low()
{
    stack_push(get_low_byte(cpu_registers.pc));
}


void cpu_push_sr()
{
    stack_push(cpu_registers.sr.u8);
}


/**
 * Fetches high byte of IRQ/BRK vector address
 */
void cpu_fetch_irq_brk_high()
{
    addr_bus = IRQ_VEC_HI;
    read();
    set_high_byte(&addr_latch, data_bus);
}


/**
 * Fetches low byte of IRQ/BRK vector address and sets PC
 */
void cpu_fetch_irq_brk_low()
{
    addr_bus = IRQ_VEC_LO;
    read();
    set_high_byte(&addr_latch, data_bus);
    cpu_registers.pc = addr_latch;
}


/**
 * ======================== RTI cycles ==============================
 */


/**
 * Does a dummy read on the stack address and then increments
 */
void cpu_inc_sp()
{
    addr_bus = sp_to_address(cpu_registers.sp);
    cpu_registers.sp++;
    read();
}


void cpu_pop_with_inc()
{
    addr_bus = sp_to_address(cpu_registers.sp);
    cpu_registers.sp++;
    read();
}


void cpu_pop_sans_inc()
{
    addr_bus = sp_to_address(cpu_registers.sp);
    read();
}


void cpu_pop_and_store_sr()
{
    cpu_registers.sr.u8 = data_bus;
    addr_bus = sp_to_address(cpu_registers.sp);
    cpu_registers.sp++;
    read();
}


void cpu_pop_and_store_()
{
    cpu_registers.sr.u8 = data_bus;
    addr_bus = sp_to_address(cpu_registers.sp);
    cpu_registers.sp++;
    read();
}


/**
 * =================== Setting register cycles ======================
 */
void cpu__set_i() { cpu_registers.sr.i = 1; }
void cpu__set_c() { cpu_registers.sr.c = 1; }
void cpu__set_v() { cpu_registers.sr.v = 1; }
void cpu__set_d() { cpu_registers.sr.d = 1; }
void cpu__clear_i() { cpu_registers.sr.i = 0; }
void cpu__clear_c() { cpu_registers.sr.c = 0; }
void cpu__clear_v() { cpu_registers.sr.v = 0; }
void cpu__clear_d() { cpu_registers.sr.d = 0; }
void cpu__txs() { cpu_registers.sp = cpu_registers.idx_x; }
void cpu__inx() { cpu_registers.sp = cpu_registers.idx_x; }

/**
 * CPU opcode decode step. CPU always executes an instruction fetch operation while this occurs (and possibly throws the result away).
 */
//void cpu__decode()
//{
//    assert(cpu__queue__size == 0);
//    switch (ir) {
//        case SEI:
//            _cpu__push(&cpu_fetch_instr, &cpu__sei, true);  // Implied addressing pattern
//            _cpu__push(&cpu_fetch_instr, &cpu__decode, false);
//            break;
//        case CLD:
//            _cpu__push(&cpu_fetch_instr, &cpu__cld, true);
//            _cpu__push(&cpu_fetch_instr, &cpu__decode, false);
//            break;
//        case CLI:
//            _cpu__push(&cpu_fetch_instr, &cpu__cli, true);
//            _cpu__push(&cpu_fetch_instr, &cpu__decode, false);
//            break;
//        case TXS:
//            _cpu__push(&cpu_fetch_instr, &cpu__txs, true);
//            _cpu__push(&cpu_fetch_instr, &cpu__decode, false);
//            break;
//        case INX:
//            _cpu__push(&cpu_fetch_instr, &cpu__inx, true);
//            _cpu__push(&cpu_fetch_instr, &cpu__decode, false);
//            break;
//        case LDX_IMM:
//            _cpu__push(&cpu_fetch_instr, &cpu_ldx_imm, true);
//            _cpu__push(&cpu_fetch_instr, &cpu__decode, false);
//            cpu_registers.pc++;
//            break;
//        case LDX_ABS:
//            _cpu__push(&cpu_fetch_instr, &cpu_parse_addr_lo, true);  // Fetch high byte, parse low byte
//            _cpu__push(&cpu_read_addr, &cpu_parse_addr_hi, true);  //
//        case STX_ABS:
//            _cpu__push(&cpu_fetch_instr, &cpu_null_exec, true);
//            _cpu__push(&cpu_fetch_addr_hi);  // does fetch for next instr TODO: Not cycle accurate!
//            _cpu__push(&cpu_stx);
//            _cpu__push(&cpu_fetch_instr, &cpu__decode);
//            break;
//        case BIT_ABS:
//            _cpu__push(&cpu_parse_addr_abs_lo);
//            _cpu__push(&cpu_parse_addr_abs_hi);  // TODO:
//            _cpu__push(&cpu_read_from_addr);
//            _cpu__push(&cpu_bit);
//            _cpu__push(&cpu_fetch_instr, &cpu__decode);
//        case BPL:
//            _cpu__push(&cpu_fetch_instr_no_inc);
//            _cpu__push(&cpu_fetch_instr, &cpu__decode);
//        default:
//            exit(ERROR_UNKNOWN_INSTR);
//    }
//}
//
//void cpu__stx()
//{
//    data_bus = cpu_registers.idx_x;
//}
//
//void cpu_read_from_addr()
//{
//    read(addr);
//}
//
//void cpu_inc_pc()
//{
//    cpu_registers.pc++;
//}
//
//void cpu_nop()
//{
//}
//
//void cpu_ldx_imm()
//{
//    cpu_registers.idx_x = ir;
//}
//
//void cpu_bit()
//{
//    cpu_registers.sr.z = ((data_bus & cpu_registers.acc) == 0);
//    cpu_registers.sr.v = (data_bus >> 6) & 1;
//    cpu_registers.sr.n = (data_bus >> 7) & 1;
//}
//
//
//void cpu_branch(uint8_t opcode)
//{
//    _cpu__push(&cpu_fetch_instr);  // Fetch operand
//    switch (opcode >> 5) {
//        case 0:
//            _cpu__push(&cpu_bpl);
//            break;
//        case 1:
//            _cpu__push(&cpu_bmi);
//            break;
//        case 2:
//            _cpu__push(&cpu_bvc);
//            break;
//        case 3:
//            _cpu__push(&cpu_bvs);
//            break;
//        case 4:
//            _cpu__push(&cpu_bcc);
//            break;
//        case 5:
//            _cpu__push(&cpu_bcs);
//            break;
//        case 6:
//            _cpu__push(&cpu_bne);
//            break;
//        case 7:
//            _cpu__push(&cpu_beq);
//            break;
//    }
//}

void cpu_ld(uint8_t opcode)
{
}
