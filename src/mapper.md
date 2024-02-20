# Intro
The NES allows many ways for a cartridge to map the CPU and PPU address space to hardware.
There is also the possibility for cartridge circuitry and bank switching to boost capacity.
The following is a list of all the indexed mapping systems that specify how each section of CPU and PPU memory address space is used.

# Mapper 0 (NROM)
Stores 16 or 32 KB of PRG ROM and 8 KB of CHR ROM.
- CPU `$8000-$BFFF`: First 16 KB of PRG ROM.
- CPU `$C000-$FFFF`: Last 16 KB of PRG ROM or mirror of `$8000-$BFFF`.
- PPU `$0000-$1FFF`: 8 KB of CHR ROM?