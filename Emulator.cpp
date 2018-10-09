//
// Created by Taylor Whatley on 2018-09-18.
//

#include "Nem.h"
#include "Errors.h"

#include <sstream>

namespace Nem {
    Emulator::Emulator(string pathToRom) {
        rom = new ROM(pathToRom);

        masterClock = new Clock();

        cpu = new CPU(masterClock, rom);
        ppu = new PPU(masterClock, rom);

        cpu->setPPU(ppu);
        ppu->setCPU(cpu);
    }

    Emulator::~Emulator() {
        cpu->stopExec();

        delete ppu;
        delete cpu;
        delete masterClock;
        delete rom;
    }
}