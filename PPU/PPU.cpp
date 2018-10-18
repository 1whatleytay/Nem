//
// Created by Taylor Whatley on 2018-09-20.
//

#include "PPU.h"

#include "../CPU/CPU.h"
#include "../Util/Clock.h"

#include <iostream>

namespace Nem {
//    void PPU::nextFrame() {
//        if (isControlSet(PPURegisters::ControlFlags::VBlankNMI)) cpu->nmi = true;
//
//        registers->status |= 0b10000000;
//        masterClock->ppuReady(360);
//        registers->status &= ~0b10000000;
//
//        oddFrame = !oddFrame;
//    }
//
//    void PPU::waitUntilNextFrame() {
//        while (!masterClock->ppuReady(88400 + !oddFrame)) { }
//    }

    bool PPU::isControlSet(PPURegisters::ControlFlags flags) {
        return (registers->control & flags) == flags;
    }
    bool PPU::isMaskSet(PPURegisters::MaskFlags flags) {
        return (registers->mask & flags) == flags;
    }

    void PPU::waitCycles(long long cycles) {
        masterClock->waitUntilPPUReady(cycles);
    };

    void PPU::postNMI() { cpu->postNMI(); }

//    void PPU::waitUntilRendering() { while (!isRendering) { } }
//
//    void PPU::finishRendering() {
//        if (!isRendering) {
//            std::cout << "Rendering was too slow!" << std::endl;
//        }
//        isRendering = false;
//    }

    void PPU::setCPU(Nem::CPU* nCPU) { cpu = nCPU; }

//    void PPU::exec() {
//        masterClock->startPPU();
//        while (!stopExecution) {
//            // Rendering
//            //Stopwatch watch;
//            //watch.start();
//            isRendering = true;
//            registers->status |= PPURegisters::StatusFlags::SprZeroHit;
//            masterClock->waitUntilPPUReady(240 * 341 - oddFrame);
//            registers->status &= ~PPURegisters::StatusFlags::SprZeroHit;
//            isRendering = false;
//            //std::cout << "Time: " << watch.stop() << std::endl;
//
//            // VBlank
//            if (isControlSet(PPURegisters::ControlFlags::VBlankNMI)) cpu->postNMI();
//            registers->status |= PPURegisters::StatusFlags::VBlankStart;
//            masterClock->waitUntilPPUReady(22 * 341);
//            registers->status &= ~PPURegisters::StatusFlags::VBlankStart;
//
//            oddFrame = !oddFrame;
//        }
//    }
//    void PPU::stopExec() { stopExecution = true; }

    PPU::PPU(Clock* nMasterClock, Mapper* mapper) : masterClock(nMasterClock) {
        memory = new PPUMemory(mapper);
        registers = new PPURegisters();
    }

    PPU::~PPU() {
        delete memory;
        delete registers;
    }

    const float c = 255.0f;

    Color palette2C02[] = {
            { 84.0f/c, 84.0f/c, 84.0f/c }, // PL 0x00
            { 0.0f/c, 30.0f/c, 116.0f/c }, // PL 0x01
            { 8.0f/c, 16.0f/c, 155.0f/c }, // PL 0x02
            { 48.0f/c, 0.0f/c, 136.0f/c }, // PL 0x03
            { 68.0f/c, 0.0f/c, 100.0f/c }, // PL 0x04
            { 92.0f/c, 0.0f/c, 48.0f/c }, // PL 0x05
            { 84.0f/c, 4.0f/c, 0.0f/c }, // PL 0x06
            { 60.0f/c, 24.0f/c, 0.0f/c }, // PL 0x07
            { 32.0f/c, 42.0f/c, 0.0f/c }, // PL 0x08
            { 8.0f/c, 58.0f/c, 0.0f/c }, // PL 0x09
            { 0.0f/c, 64.0f/c, 0.0f/c }, // PL 0x0A
            { 0.0f/c, 60.0f/c, 0.0f/c }, // PL 0x0B
            { 0.0f/c, 50.0f/c, 60.0f/c }, // PL 0x0C
            { 0.0f/c, 0.0f/c, 0.0f/c }, // PL 0x0D
            { 0.0f/c, 0.0f/c, 0.0f/c }, // PL 0x0E
            { 0.0f/c, 0.0f/c, 0.0f/c }, // PL 0x0F
            { 152.0f/c, 150.0f/c, 152.0f/c }, // PL 0x10
            { 8.0f/c, 76.0f/c, 196.0f/c }, // PL 0x11
            { 48.0f/c, 50.0f/c, 236.0f/c }, // PL 0x12
            { 92.0f/c, 30.0f/c, 228.0f/c }, // PL 0x13
            { 136.0f/c, 20.0f/c, 176.0f/c }, // PL 0x14
            { 160.0f/c, 20.0f/c, 100.0f/c }, // PL 0x15
            { 152.0f/c, 34.0f/c, 32.0f/c }, // PL 0x16
            { 120.0f/c, 60.0f/c, 0.0f/c }, // PL 0x17
            { 84.0f/c, 90.0f/c, 0.0f/c }, // PL 0x18
            { 40.0f/c, 114.0f/c, 0.0f/c }, // PL 0x19
            { 8.0f/c, 124.0f/c, 0.0f/c }, // PL 0x1A
            { 0.0f/c, 118.0f/c, 40.0f/c }, // PL 0x1B
            { 0.0f/c, 102.0f/c, 120.0f/c }, // PL 0x1C
            { 0.0f/c, 0.0f/c, 0.0f/c }, // PL 0x1D
            { 0.0f/c, 0.0f/c, 0.0f/c }, // PL 0x1E
            { 0.0f/c, 0.0f/c, 0.0f/c }, // PL 0x1F
            { 236.0f/c, 238.0f/c, 236.0f/c }, // PL 0x20
            { 76.0f/c, 154.0f/c, 236.0f/c }, // PL 0x21
            { 120.0f/c, 124.0f/c, 236.0f/c }, // PL 0x22
            { 176.0f/c, 98.0f/c, 236.0f/c }, // PL 0x23
            { 228.0f/c, 84.0f/c, 236.0f/c }, // PL 0x24
            { 236.0f/c, 88.0f/c, 180.0f/c }, // PL 0x25
            { 236.0f/c, 106.0f/c, 100.0f/c }, // PL 0x26
            { 212.0f/c, 136.0f/c, 32.0f/c }, // PL 0x27
            { 160.0f/c, 170.0f/c, 0.0f/c }, // PL 0x28
            { 116.0f/c, 196.0f/c, 0.0f/c }, // PL 0x29
            { 76.0f/c, 208.0f/c, 32.0f/c }, // PL 0x2A
            { 56.0f/c, 204.0f/c, 108.0f/c }, // PL 0x2B
            { 56.0f/c, 180.0f/c, 204.0f/c }, // PL 0x2C
            { 60.0f/c, 60.0f/c, 60.0f/c }, // PL 0x2D
            { 0.0f/c, 0.0f/c, 0.0f/c }, // PL 0x2E
            { 0.0f/c, 0.0f/c, 0.0f/c }, // PL 0x2F
            { 236.0f/c, 238.0f/c, 236.0f/c }, // PL 0x30
            { 168.0f/c, 204.0f/c, 236.0f/c }, // PL 0x31
            { 188.0f/c, 188.0f/c, 236.0f/c }, // PL 0x32
            { 212.0f/c, 178.0f/c, 236.0f/c }, // PL 0x33
            { 236.0f/c, 174.0f/c, 236.0f/c }, // PL 0x34
            { 236.0f/c, 174.0f/c, 212.0f/c }, // PL 0x35
            { 236.0f/c, 180.0f/c, 176.0f/c }, // PL 0x36
            { 228.0f/c, 196.0f/c, 144.0f/c }, // PL 0x37
            { 204.0f/c, 210.0f/c, 120.0f/c }, // PL 0x38
            { 180.0f/c, 222.0f/c, 120.0f/c }, // PL 0x39
            { 168.0f/c, 226.0f/c, 144.0f/c }, // PL 0x3A
            { 152.0f/c, 226.0f/c, 180.0f/c }, // PL 0x3B
            { 160.0f/c, 214.0f/c, 228.0f/c }, // PL 0x3C
            { 160.0f/c, 162.0f/c, 160.0f/c }, // PL 0x3D
            { 0.0f/c, 0.0f/c, 0.0f/c }, // PL 0x3E
            { 0.0f/c, 0.0f/c, 0.0f/c }, // PL 0x3F
    };
}