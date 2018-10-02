cmake_minimum_required(VERSION 3.12)
project(Nem)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_AUTOMOC ON)

find_package(Qt5Widgets CONFIG REQUIRED)
find_package(Qt5OpenGL CONFIG REQUIRED)

find_package(OpenGL REQUIRED)

if (APPLE)
    add_definitions(-DGL_SILENCE_DEPRECATION)
endif()

set(Sources
        Qt/main.cpp
        Qt/QNem.cpp

        Emulator.h
        Emulator.cpp

        Internal.h
        Internal.cpp

        ROM/ROM.h
        ROM/ROM.cpp

        CPU/CPU.h
        CPU/CPU.cpp
        CPU/CPUMemory.cpp
        CPU/Operations.cpp
        CPU/Codes.cpp

        PPU/PPU.h
        PPU/PPU.cpp
        PPU/PPUMemory.cpp
        Qt/QNemWindow.cpp Qt/QNemWindow.h Nem.h Timer.cpp Timer.h)

set(Shaders
        Shaders/map.vert
        Shaders/sprite.vert
        Shaders/paletted.frag)

configure_file(Shaders/map.vert ${CMAKE_CURRENT_BINARY_DIR}/map.vert COPYONLY)
configure_file(Shaders/sprite.vert ${CMAKE_CURRENT_BINARY_DIR}/sprite.vert COPYONLY)
configure_file(Shaders/paletted.frag ${CMAKE_CURRENT_BINARY_DIR}/paletted.frag COPYONLY)

add_executable(Nem ${Sources})
target_link_libraries(Nem ${OPENGL_LIBRARIES} Qt5::Widgets Qt5::OpenGL)