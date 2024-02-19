cmake_minimum_required(VERSION 3.27)
project(tiny_emulator C)

set(CMAKE_C_STANDARD 17)

add_executable(tiny_emulator main.c
        src/cpu/instructions.h
        src/cpu/cpu.h
        src/ram/ram.h
        src/cpu/cpu.c
        src/clock.c
        src/clock.h
        src/ram/ram.c
        src/io.c
        src/io.h
        src/mock_rom/mock_rom.h
        src/load.c
        src/load.h)
