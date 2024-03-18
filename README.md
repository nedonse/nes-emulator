# Intro
The MOS6502 is a little-endian 8-bit processor with a 16-bit address memory bus.

It features 2 KB of internal RAM which is mirrored 3 times to occupy the addresses `$0000-$1FFF`. The remaining address
space is reserved for I/O, PPU cpu_registers, and other buffers.

The MOS6502 processor has a handful of cpu_registers:
- `pc`: 16-bit program counter.
- `sp`: 8-bit stack pointer. The stack pointer is only an 8-bit address, and is interpreted as an offset from `$0100`, which is the start of the 256-byte page reserved for the stack.
- `acc`: 8-bit accumulator. Used for arithmetic operations.
- `x`: 8-bit indexing register.
- `y`: 8-bit secondary indexing register.
- `sr`: 8-bit status register. Contains 7 flags. From lowest to highest bit,
  - `c`: carry flag. Used in addition, subtraction, comparison, and rotation operations.
  - `z`: zero flag.
  - `i`: interrupt disable flag. Triggers after handling an interrupt to prevent processor from repeatedly executing interrupt routine if `IRQ` signal is low for multiple cycles.
  - `d`: decimal flag. Refer to [reference](https://www.nesdev.org/6502_cpu.txt).
  - `b`: break flag. Distinguishes software interrupts (i.e., `BRK`) from hardware interrupts from `IRQ` or `NMI`. Refer to [reference](https://www.nesdev.org/6502_cpu.txt).
  - `_`: unused. Always 1.
  - `v`: overflow flag.
  - `n`: negative flag.

# Memory Types
There are several main uses of memory: PRG ROM, PRG RAM, CHR ROM, and CHR RAM.

# CPU Memory Layout
Main memory `$0000-$1FFF` contains two special features.
- The first 256 bytes `$0000-$00FF` are the zero page, which has special access instructions that require fewer cycles and bytes to access.
- The page `$0100-$01FF` is reserved for the stack.

Then, the NES PPU cpu_registers lie in `$2000-$2008`, which are mirrored many times to take up the entirety of `$2000-$3FFF`.

Then, the regularly-used NES APU and I/O cpu_registers lie in `$4000-$4017`. The remaining cpu_registers, which aren't normally used, occupy the rest of the space `$4018-$401F`.

Finally, the remaining chunk of the memory map, `$4020-$FFFF`, goes to the cartridge, which consists of PRG ROM, PRG RAM, CHR ROM, CHR RAM, and mapping cpu_registers.
(In emulation, this part of memory layout may be implemented extremely flexibly?)

The interrupt vectors must be located in the cartridge at
- `NMI`: `$FFFA-$FFFB`
- `RESET`: `$FFFC-$FFFD`
- `IRQ`/`BRK`: `$FFFE-$FFFF`

Refer [here](https://www.nesdev.org/wiki/CPU_memory_map) for further information.

# PPU Memory Layout
[https://www.nesdev.org/wiki/PPU_memory_map]()

# Cartridges
The common format for cartridges used by NES emulators is the `.nes` format, pioneered by the iNES emulator.
There are several variants and updates since the original format was defined. Currently, the most widely adopted format is iNES 2.0.

The `.nes` format is similar to a regular assembled binary format, but includes an extra 16-byte header. The header
broadcasts information about the size of PRG ROM and CHR ROM, mirroring, and mapper behavior.

Mappers are indexed according to an 8-bit unsigned integer (12-bit in iNES 2.0).
- `$0-$3`: 4 bytes for the ASCII characters `NES` (`0x4e`, `0x45`, `0x53`) followed by the MS-DOS end-of-file character `0x1A`.
- `$4`: Size of PRG ROM in multiples of 16 KB.
- `$5`: Size of CHR ROM in multiples of 8 KB. (If 0, CHR RAM is to be used.)
- `$6`: Flags. Represents mapper, mirroring, battery, and trainer.
- `$7`: Flags.
- `$8`: Flags. PRG-RAM size.
- `$9`: Flags.
- `$A`: Flags.
- `$B-F`: Unused padding. (Values could be anything, usually zero but sometimes used by a ripper to sign the ROM.)

Then, there is PRG ROM and CHR ROM.

See [this](https://www.nesdev.org/wiki/INES) for more details about the header.

After the header, the rest of the file contains PRG ROM or CHR ROM in 16 KB or 8 KB sections respectively.

Cartridges are designed to be highly flexible, enabling them to use circuits and mapping to circumvent storage and RAM size limitations, as well as other hardware capabilities.
For example, a cartridge might require far more ROM space than is available from the NES address space and instead have many more KB of ROM storage which take turns occupying the mapped address space. This behavior is managed by what is called a mapper, and since each mapper is cartridge-specific, the NES emulation community uses indexing to track different mapper types.

See [this README](rom/README.md) for how to program cartridges.

# Interrupts
## Reset
Upon power-on, the CPU jumps to the address specified by the cpu_reset vector, i.e., it sets `pc` to the value in `$FFFC-$FFFD`.

## NMI
Non-maskable interrupt. Called on the beginning of the vertical blanking period for every frame.
https://www.moria.us/blog/2018/03/nes-development

## IRQ
Signalled by the game cartridge.

# Memory Bus
Does memory access have zero wait state?

The instruction register is used to store fetched instruction bytes [source](https://www.nesdev.org/wiki/Visual6502wiki/6502_Timing_States).

# PPU
## Registers
https://www.nesdev.org/wiki/PPU_registers

# Instructions
All opcodes are 8-bit. Refer to this [file](https://www.nesdev.org/6502_cpu.txt) for detailed explanation of nuances and per-cycle behavior and this [page](https://www.nesdev.org/obelisk-6502-guide) for quick summaries of the main opcodes.

## Cycle Breakdown
On each cycle, the CPU attempts


## Addressing Modes
- Immediate: instead of address, operand is the 8-bit value to be used.
- Zero Page: 8-bit address to the zero page.
  - Zero Page,X: zero page address offset by the value of the X register. Offset computation is wrapped.
  - Zero Page,Y: zero page address offset by the value of the Y register.
- Absolute: full 16-bit address to any location in the memory map.
  - Absolute,X
  - Absolute,Y
- Relative
- Indirect
  - Indirect,X
  - Indirect,Y

See this [page](https://www.nesdev.org/wiki/CPU_addressing_modes) and [cpu_txt](https://www.nesdev.org/6502_cpu.txt)

### Crossing a Page Boundary
For some addressing modes, the effective address crosses a page boundary from the initial address. A "page", in this case, refers to blocks of 256 bytes starting from `$0000`.
Consider, for example, the following instructions

```
LDX #1
ADC $00FF,x
```
The effective address of this Absolute,X addressing is then `$0100`, which crosses the boundary. In this case, when the CPU adds the value of `x` to the low byte of the initial address, the operation will wrap and yield an incorrect address.
The CPU will then need to take an extra cycle to correct the address. Specifically, upon computing the invalid effective address, the CPU will in the next clock cycle read from that possibly invalid address and attempt to validate/fix the effective address simultaneously. If the address was indeed invalid, the CPU takes an extra cycle to read from the fixed effective address.

https://forums.nesdev.org/viewtopic.php?t=13936

# Testing
## ROM Programming
http://www.6502.org/tools/asm/

# CPU Control Signals and Timing
The below is a copy from this [link](https://retrocomputing.stackexchange.com/questions/11216/how-exactly-do-all-control-signals-in-6502-work).

The CPU has 3 control signals
- PHI0: Input system clock signal. Pulses low and high at some fixed rate.
- PHI1: Output clock signal; PHI0 but inverted and the high pulse is shortened (see bullet point below for reason).
- PHI2: Output clock signal; identical to PHI0 with the high pulse shortened (same reason as PHI1).
- When PHI1 is high, the processor is doing internal processing stuff and setting all the output pins to the correct state in prep for PHI2 to go high (except data pins)
- When PHI2 is high, the CPU does memory access.
- The high pulses are shortened to give margin between PHI1 and PHI2.
- Keep in mind that the processor _ALWAYS_ does memory access, even if it doesn't have to.

There are some other signals necessary for memory access:
- R/W: High for read, low for write.
- SYNC: High for when opcodes are being read.
- A0-15: Address bus bits
- D0-7: Data bus bits

There's also
- RDY: which, if you set it low, will cause the processor to freeze at the beginning of the next ø2 as long as it is a read cycle. If you set RDY low when SYNC is high, you can single step the processor.
- IRQ, NMI: for interrupts.
- Reset: stops processor when falling low and restarts it when rising high.
- SO: status overflow?

See also the timing diagram here: http://forum.6502.org/viewtopic.php?f=1&t=3185

And this: https://www.nesdev.org/wiki/Visual6502wiki/6502_datapath

http://forum.6502.org/viewtopic.php?f=8&t=5429

When PHI1 starts, the IR is set to the opcode if receiving an opcode.

## Intricate Details
It is best to reference the 6502 block diagram for this section. It is not completely accurate but is overall helpful.

### Predecode Register
Loads from the data bus every PHI2 rising edge.

### Predecode Logic
Before decoding the incoming byte from the Predecode Register, it decides whether the PC should increment or not (i.e., if it is a single byte instruction).

https://retrocomputing.stackexchange.com/questions/4880/6502-what-does-the-predecode-register-exactly-do

### 

# How the CPU decodes and executes instructions
Stolen from here: https://www.pagetable.com/?p=39

See also: https://www.nesdev.org/wiki/Visual6502wiki/6502_State_Machine

There's many "KIL" opcodes that stall the CPU such that only a reset can recover it.

https://www.nesdev.org/wiki/Visual6502wiki/6502_Timing_States
https://www.nesdev.org/wiki/Visual6502wiki/6502_all_256_Opcodes

# Interrupts
https://www.nesdev.org/wiki/Visual6502wiki/6502_Interrupt_Recognition_Stages_and_Tolerances

# Terminology
Control Line: any logic line that controls a logic device, e.g., register output enable, register load, register asynchronous clear, counter increment, counter reset, counter decrement, memory write-enable, tri-state buffer output-enable, bus-transceiver direction, ALU function selection lines, etc. Usually there are dozens of these control lines all over a CPU and the point of instruction decoding is to set the state of these control lines based on the current instruction and other status lines (e.g., flags for the conditional branches, etc.)

Instruction Decoding: an instruction word is loaded into the instruction register, and that plus various status lines, and finally a micro-instruction counter will be the input of a state-control table. Input is the state lines and output is the control lines.

When I say "no decoding" in this context means that there is no need for such a state-control table, because the instruction word is comprised plainly of the bits of the control lines, but especially the number of microinstruction cycles is 1 or constant, not variable. I.e., every instruction takes the same number of cycles, without wasting any cycles either.

Cycle - a clock cycle, consisting of at least 2 phases, and 2 instantaneous moments. They are: rising edge (instantaneous moment), high state of clock (phase), falling edge (instantaneous moment), low state of clock.

Phase - given that any square wave clock signal has these 2 phases (high, and low) and their rising and falling edge moments, many processors require derivative signals. Sometimes called phi-2 or phi' (prime), etc. These are obviously necessary to subdivide a single clock cycle into multiple sub-steps in which different things happen.

An example for different things happening in the phases and moments of a normal square wave clock: rising edge: registers clock in the current state of the data bus or internal data bus, synchronous counters step at the rising edge. Falling edge: that's where a memory address latch may hold the address lines on a shared address/data bus so that in the low phase of the clock signal the data from memory appears on the bus while the address is held in the latch.

Microinstruction Cycle: when instructions are executed in multiple micro-instruction steps, for example:

IR = *IP // load instruction from instruction pointer
DP = *IP // load absolute address into data pointer
DP = *DP + Y // resolve indirect address adding the Y index
AC = AC + *DP + C // add with carry data from memory to accumulator register
Stuff like that.