cmake_minimum_required(VERSION 3.27)
project(nes_emulator C)

set(CMAKE_C_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

include_directories(src)

add_executable(nes_emulator main.c
        src/cpu/cpu.h
        src/ram/ram.h
        src/cpu/cpu.c
        src/clock.c
        src/clock.h
        src/ram/ram.c
        src/io.c
        src/io.h
        src/load.c
        src/load.h
        src/ppu.c
        src/ppu.h
        src/utils.c
        src/utils.h)
