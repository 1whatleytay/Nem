//
// Created by Taylor Whatley on 2018-09-19.
//

#include "CPU.h"
#include "../Clock/Clock.h"
#include "Codes.h"

#include <iostream>

namespace Nem {
#ifdef MARIO_8057
    bool reached8057 = false;

    void CPU::wait8057() {
        while(!reached8057) { }
    }
#endif

    void CPU::processIRQ() {

    }
    void CPU::processNMI() {
        if (nmi) {
            std::cout << "BA: " << makeHex(registers->programCounter) << std::endl;
            pushAddress(registers->programCounter);
            pushByte(registers->status);
            registers->programCounter = memory->getNMIVector();
            registers->programCounter--;
#ifdef MARIO_8057
            if (!reached8057) std::cout << "DID NOT REACH 8057!" << std::endl;
#endif
#ifdef RESET_NMI
            nmi = false;
#endif
        }
    }

    Byte CPU::nextByte(Address next) {
        return memory->getByte(registers->programCounter + next);
    }

    Address CPU::nextAddress(Address next) {
        return memory->getAddress(registers->programCounter + next);
    }

    bool CPU::isFlagSet(Nem::CPURegisters::StatusFlags flags) {
        return (registers->status & flags) == flags;
    }

    void CPU::clearFlags(Nem::CPURegisters::StatusFlags flags) {
        registers->status &= ~flags;
    }

    void CPU::setFlags(Nem::CPURegisters::StatusFlags flags) {
        registers->status |= flags;
    }

    void CPU::setFlags(bool condition, Nem::CPURegisters::StatusFlags flags) {
        if (condition) setFlags(flags);
        else clearFlags(flags);
    }

    Address stackLocation = 0x0100;

    void CPU::pushByte(Byte byte) {
        memory->lockStack = false;
        memory->setByte(stackLocation + registers->stackPointer, byte);
        registers->stackPointer--;
        memory->lockStack = true;
    }
    void CPU::pushAddress(Address address) {
        memory->lockStack = false;
        memory->setAddress(stackLocation + registers->stackPointer - (Address)1, address);
        registers->stackPointer -= 2;
        memory->lockStack = true;
    }
    Byte CPU::popByte() {
        memory->lockStack = false;
        registers->stackPointer++;
        Byte byte = memory->getByte(stackLocation + registers->stackPointer);
        memory->lockStack = true;
        return byte;
    }
    Address CPU::popAddress() {
        memory->lockStack = false;
        registers->stackPointer += 2;
        Address address = memory->getAddress(stackLocation + registers->stackPointer - (Address)1);
        memory->lockStack = true;
        return address;
    }

    void CPU::setPPU(PPU* nPPU) { memory->setPPU(nPPU); }

    void CPU::step() {
        Address index = ++registers->programCounter;

#ifdef MARIO_8057
        reached8057 = true;
#endif
        Byte instructionId = memory->getByte(index);
        Instruction instruction = opFunctions[instructionId];

#ifdef PRINT_INSTRUCTIONS
        outFile << makeHex(registers->programCounter) << ", " << makeHex(instructionId) << ", " << opNames[instructionId] << ", "
        << makeHex(registers->accumulator) << ", " << makeHex(registers->indexX) << ", " << makeHex(registers->indexY) << ", "
        << makeHex(registers->status) << ", " << makeHex(registers->stackPointer) << ", " << cycles << "\n";
#endif
        long long cyclesBefore = cycles;

        int length = instruction(this);
        cycles += 2; // Every instruction usually takes at least 2 cycles.

        while (!masterClock->cpuReady(cycles - cyclesBefore)) { }

        registers->programCounter += length;

        processIRQ();
        processNMI();
    }

    void CPU::exec() {
        while (!stopExecution) { step(); }
    }
    void CPU::stopExec() {
        stopExecution = true;
    }

    CPU::CPU(Clock* nMasterClock, ROM* rom) : masterClock(nMasterClock) {
        memory = new CPUMemory(rom);
        registers = new CPURegisters();

#ifdef FORCE_ENTRY
        registers->programCounter = FORCE_ENTRY;
#else
        registers->programCounter = memory->getResetVector();
#endif
        registers->programCounter--;
    }

    CPU::~CPU() {
        delete memory;
        delete registers;

#ifdef PRINT_INSTRUCTIONS
      outFile.close();
#endif
    }
}