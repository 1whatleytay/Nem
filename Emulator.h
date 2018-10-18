//
// Created by Taylor Whatley on 2018-09-18.
//

#ifndef NEM_EMULATOR_H
#define NEM_EMULATOR_H

#include "Internal.h"
#include "Errors.h"

/**
 * Sources:
 *
 * http://wiki.nesdev.com/w/index.php/CPU_unofficial_opcodes
 * http://wiki.nesdev.com/w/index.php/CPU_status_flag_behavior
 * http://wiki.nesdev.com/w/index.php/CPU_addressing_modes
 * https://wiki.nesdev.com/w/index.php/Stack
 * http://www.6502.org/tutorials/6502opcodes.html
 * https://www.c64-wiki.com/wiki/BRK
 */

/**
 * - Set VBL Flag Correctly
 * - Implement Grayscale
 * - Implement Emphasis
 */

namespace Nem {
    class ROM;
    class CPU;
    class PPU;
    class APU;
    class Clock;
    class Mapper;
    class ControllerInterface;

    class Emulator {
    public:
        ROM* rom = nullptr;
        Mapper* mapper = nullptr;

        Clock* masterClock = nullptr;

        CPU* cpu = nullptr;
        PPU* ppu = nullptr;
        APU* apu = nullptr;

        void setController(int index, ControllerInterface* controller);

        explicit Emulator(string pathToRom);
        ~Emulator();
    };
}

#endif //NEM_EMULATOR_H
