//
// Created by Taylor Whatley on 2018-09-19.
//

#include "CPU.h"
#include "CPUCodes.h"

#include "../Util/Clock.h"
#ifdef NEM_PROFILE
#include "../Debug/Profiler.h"
#endif

#include <iostream>
#include <unordered_map>

#ifndef CPU_ENTRY
#define CPU_ENTRY memory.getResetVector()
#endif

namespace Nem {
    std::unordered_map<string, bool> debugFlags;

    bool getDebugFlag(string name) { return debugFlags[name]; }
    void setDebugFlag(string name, bool value) { debugFlags[name] = value; }

    CPU* debugCPU = nullptr;

    Address getDebugPC() {
        return debugCPU->registers.programCounter;
    }

    void CPU::waitCycles(long long cycles) { masterClock->waitUntilCPUReady(cycles); }

    void CPU::readCycle() {
        cycles++;
        waitCycles(1);
    }
    Byte CPU::readCycle(Byte value) { readCycle(); return value; }
    void CPU::writeCycle() {
        cycles++;
        waitCycles(1);
    }

    void CPU::processIRQ() {
        if (irq) {
            std::cout << "IRQ Sent" << std::endl;
            irq = false;
        }
    }
    void CPU::processNMI() {
        if (nmi) {
            pushAddress(registers.programCounter);
            pushByte(registers.status);
            registers.programCounter = memory.getNMIVector();
            registers.programCounter--;
            nmi = false;
        }
    }

    void CPU::postIRQ() { irq = true; }
    void CPU::postNMI() { nmi = true; }

    Byte CPU::thisByte(bool cycle) {
        return memory.getByte(registers.programCounter, cycle);
    }

    Byte CPU::nextByte(bool cycle) {
        return memory.getByte(registers.programCounter + (Address)1, cycle);
    }

    Address CPU::nextAddress(bool cycle) {
        return memory.getAddress(registers.programCounter + (Address)1, cycle);
    }

    bool CPU::isFlagSet(Nem::CPURegisters::StatusFlags flags) {
        return (registers.status & flags) == flags;
    }

    void CPU::clearFlags(Nem::CPURegisters::StatusFlags flags) {
        registers.status &= ~flags;
    }

    void CPU::setFlags(Nem::CPURegisters::StatusFlags flags) {
        registers.status |= flags;
    }

    void CPU::setFlags(bool condition, Nem::CPURegisters::StatusFlags flags) {
        if (condition) setFlags(flags);
        else clearFlags(flags);
    }

    Address stackLocation = 0x0100;

    void CPU::pushByte(Byte byte) {
        memory.setByte(stackLocation + registers.stackPointer, byte);
        registers.stackPointer--;
    }
    void CPU::pushAddress(Address address) {
        memory.setAddress(stackLocation + registers.stackPointer - (Address)1, address);
        registers.stackPointer -= 2;
    }
    Byte CPU::popByte() {
        registers.stackPointer++;
        Byte byte = memory.getByte(stackLocation + registers.stackPointer);
        return byte;
    }
    Address CPU::popAddress() {
        registers.stackPointer += 2;
        Address address = memory.getAddress(stackLocation + registers.stackPointer - (Address)1);
        return address;
    }

    void CPU::setPPU(PPU* nPPU) { memory.setPPU(nPPU); }
    void CPU::setAPU(APU *nAPU) { memory.setAPU(nAPU); }
    void CPU::setController(int index, ControllerInterface* controller) { memory.setController(index, controller); }

    void CPU::step() {
        registers.programCounter++;

#ifdef NEM_PROFILE
        profiler->analyzeStep();
#endif

        Byte opCode = memory.getByte(registers.programCounter);
        AddressedInstruction instruction = opInstructions[opCode];

        int length = callAddressMode(instruction, opModes[opCode], this);

        registers.programCounter += length - 1;

        processIRQ();
        processNMI();
    }

    void CPU::exec() {
        masterClock->startCPU();
        while (!stopExecution) { step(); }
    }
    void CPU::stopExec() {
        stopExecution = true;
    }

    CPU::CPU(Clock* nMasterClock, Mapper* mapper) : masterClock(nMasterClock), memory(this, mapper) {
        registers.programCounter = CPU_ENTRY;
        registers.programCounter--;

#ifdef NEM_PROFILE
        profiler = new Profiler(this);
#endif

        debugCPU = this;
    }

    CPU::~CPU() {
#ifdef NEM_PROFILE
        delete profiler;
#endif
    }
}