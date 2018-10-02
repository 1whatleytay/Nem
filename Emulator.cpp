//
// Created by Taylor Whatley on 2018-09-18.
//

#include "Nem.h"
#include "Errors.h"

#include <sstream>

namespace Nem {
    Emulator::Emulator(string& pathToRom) {
        rom = new ROM(pathToRom);

        cpu = new CPU(rom);
        ppu = new PPU(rom);

        cpu->setPPU(ppu);
        ppu->setCPU(cpu);
    }

    Emulator::~Emulator() {
        delete rom;
        delete cpu;
        delete ppu;
    }
}