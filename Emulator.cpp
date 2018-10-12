//
// Created by Taylor Whatley on 2018-09-18.
//

#include "Emulator.h"

#include "CPU/CPU.h"
#include "PPU/PPU.h"
#include "Mapper/Mapper.h"
#include "Clock/Clock.h"

namespace Nem {
    void Emulator::setController(int index, ControllerInterface* controller) {
        cpu->setController(index, controller);
    }

    Emulator::Emulator(string pathToRom) {
        rom = new ROM(pathToRom);
        mapper = mappers[rom->header.getMapper()](rom);

        if (mapper == nullptr) throw RomUnimplementedException(
                "mapper " + mapperNames[rom->header.getMapper()] + " : " +
                std::to_string((int)rom->header.getMapper()));

        masterClock = new Clock();

        cpu = new CPU(masterClock, mapper);
        ppu = new PPU(masterClock, mapper);

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