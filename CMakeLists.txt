cmake_minimum_required(VERSION 3.12)
project(Nem)

set(CMAKE_CXX_STANDARD 11)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread -O1 -fno-omit-frame-pointer -g")

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(glfw)
add_subdirectory(gl3w)

if (APPLE)
    add_definitions(-DGL_SILENCE_DEPRECATION)
endif()

#include_directories(gl3w/include)

set(Sources

        Internal.h
        Internal.cpp

        Emulator.h
        Emulator.cpp

        ROM/ROM.h
        ROM/ROM.cpp

        Mapper/Mapper.h
        Mapper/Mapper.cpp

        CPU/CPU.h
        CPU/CPU.cpp
        CPU/CPUMemory.cpp
        CPU/CPUCodes.h
        CPU/CPUCodes.cpp

        PPU/PPU.h
        PPU/PPU.cpp
        PPU/PPUMemory.cpp

        APU/APU.h
        APU/APU.cpp

        Controller/Controller.h
        Controller/Controller.cpp

        Util/Clock.h
        Util/Clock.cpp
        Util/Stopwatch.h
        Util/Stopwatch.cpp
        Util/Ranges.h
        Util/Ranges.cpp

        Debug/Profiler.h
        Debug/Profiler.cpp
        Debug/Disassembler.h
        Debug/Disassembler.cpp)

set(SourcePPU

        App/App.h
        App/App.cpp
        App/Display.cpp)

add_executable(Nem ${Sources} ${SourcePPU} main.cpp)
target_link_libraries(Nem glfw gl3w)

add_executable(NemCPU ${Sources} main.cpp)
target_compile_definitions(NemCPU PRIVATE CPU_ONLY)