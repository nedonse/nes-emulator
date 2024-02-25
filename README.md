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