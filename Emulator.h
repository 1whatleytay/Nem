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
 * [FAIL] Fix PPUADDR and PPUDATA (working writes, PPUADDR increments by the right amount)
 * [FAIL] Make and load shaders.
 * [FAIL] Create background drawing.
 * [FAIL] Create OAM space.
 * [FAIL] Implement OAM registers.
 * [FAIL] Create Sprite drawing.
 * [FAIL] Different PPU Cycles are accounted.
 * [FAIL] CPU timing is good (nestest.nes cyc compliance).
 * [PASS] NMI for PPU.
 */

namespace Nem {
    class ROM;
    class CPU;
    class PPU;

    class Emulator {
    public:
        ROM* rom = nullptr;
        CPU* cpu = nullptr;
        PPU* ppu = nullptr;

        explicit Emulator(string& pathToRom);
        ~Emulator();
    };
}

#endif //NEM_EMULATOR_H
