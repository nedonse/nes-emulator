https://www.nesdev.org/wiki/CPU_memory_map

Internal RAM

 - 2 KB mirrored 4 times
 - $0000-$00FF: Zero page which is accessed with fewer bytes and cycles
 - $0100-$01FF: Page containing the stack, which could be located anywhere, but typically starts at $01FF
   - sp is 1 byte, which is just enough for this
   - sp represents offset from $0100, so memory is $0100+sp