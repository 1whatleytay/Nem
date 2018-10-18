//
// Created by Taylor Whatley on 2018-09-19.
//

#include "CPU.h"
#include "../Util/Clock.h"
#include "CPUCodes.h"

#include <iostream>

namespace Nem {
    CPU* debugCPU = nullptr;

    Address getDebugPC() {
        return debugCPU->registers->programCounter;
    }

    void CPU::waitCycles(long long cycles) { masterClock->waitUntilCPUReady(cycles); }

    void CPU::processIRQ() {

    }
    void CPU::processNMI() {
        if (nmi) {
            pushAddress(registers->programCounter);
            pushByte(registers->status);
            registers->programCounter = memory->getNMIVector();
            registers->programCounter--;
#ifdef NMI_RESET
            nmi = false;
#endif
        }
    }

    void CPU::postIRQ() { irq = true; }
    void CPU::postNMI() { nmi = true; }

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
        memory->setByte(stackLocation + registers->stackPointer, byte);
        registers->stackPointer--;
    }
    void CPU::pushAddress(Address address) {
        memory->setAddress(stackLocation + registers->stackPointer - (Address)1, address);
        registers->stackPointer -= 2;
    }
    Byte CPU::popByte() {
        registers->stackPointer++;
        Byte byte = memory->getByte(stackLocation + registers->stackPointer);
        return byte;
    }
    Address CPU::popAddress() {
        registers->stackPointer += 2;
        Address address = memory->getAddress(stackLocation + registers->stackPointer - (Address)1);
        return address;
    }

    void CPU::setPPU(PPU* nPPU) { memory->setPPU(nPPU); }
    void CPU::setAPU(APU *nAPU) { memory->setAPU(nAPU); }
    void CPU::setController(int index, ControllerInterface* controller) { memory->setController(index, controller); }

    void CPU::step() {
//#ifdef PROFILE_CPU
//        profiler.analyzeStep();
//#endif

        Address index = ++registers->programCounter;

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

        masterClock->waitUntilCPUReady(cycles - cyclesBefore);

        registers->programCounter += length;

        processIRQ();
        processNMI();
    }

    void CPU::exec() {
        masterClock->startCPU();
        while (!stopExecution) { step(); }
    }
    void CPU::stopExec() {
        stopExecution = true;
#ifdef PRINT_INSTRUCTIONS
        outFile.close();
#endif
    }

    CPU::CPU(Clock* nMasterClock, Mapper* mapper) : masterClock(nMasterClock) {
        memory = new CPUMemory(this, mapper);
        registers = new CPURegisters();

#ifdef FORCE_ENTRY
        registers->programCounter = FORCE_ENTRY;
#else
        registers->programCounter = memory->getResetVector();
#endif
        registers->programCounter--;

        debugCPU = this;
    }

    CPU::~CPU() {
        delete memory;
        delete registers;

#ifdef PRINT_INSTRUCTIONS
        outFile.close();
#endif
    }
}