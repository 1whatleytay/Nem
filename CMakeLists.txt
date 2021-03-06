cmake_minimum_required(VERSION 3.12)
project(Nem)

set(CMAKE_CXX_STANDARD 11)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -O1 -fno-omit-frame-pointer -g")

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(External/glfw)
if (WIN32)
    add_subdirectory(External/gl3w)
else()
    find_package(OpenGL REQUIRED)
endif()

add_definitions(-DGL_SILENCE_DEPRECATION)

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
        App/Display.cpp
        App/Shaders.cpp)

configure_file(Shaders/vertex.vert ${CMAKE_BINARY_DIR}/vertex.vert COPYONLY)
configure_file(Shaders/fragment.frag ${CMAKE_BINARY_DIR}/fragment.frag COPYONLY)

add_executable(Nem ${Sources} ${SourcePPU} main.cpp)
if (WIN32)
    target_link_libraries(Nem glfw gl3w)
else()
    target_link_libraries(Nem glfw OpenGL::GL)
endif()

add_executable(NemCPU ${Sources} main.cpp)
target_compile_definitions(NemCPU PRIVATE CPU_ONLY)
