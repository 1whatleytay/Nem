//
// Created by Taylor Whatley on 2018-09-19.
//

#include "CPU.h"
#include "Codes.h"

namespace Nem {
    void CPU::processIRQ() {

    }
    void CPU::processNMI() {
        if (nmi) {
            pushAddress(registers->programCounter);
            pushByte(registers->status);
            registers->programCounter = memory->getNMIVector();
            registers->programCounter--;
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

#ifdef PRINT_INSTRUCTIONS
    std::ofstream outFile = std::ofstream("/Users/desgroup/Desktop/nem.log.txt");
#endif

    void CPU::step() {
        Address index = ++registers->programCounter;
        Byte instructionId = memory->getByte(index);
        Instruction instruction = opFunctions[instructionId];
#ifdef PRINT_INSTRUCTIONS
        outFile << makeHex(registers->programCounter) << ", " << makeHex(instructionId) << ", " << opNames[instructionId] << ", "
        << makeHex(registers->accumulator) << ", " << makeHex(registers->indexX) << ", " << makeHex(registers->indexY) << ", "
        << makeHex(registers->status) << ", " << makeHex(registers->stackPointer) << ", " << cycles << "\n";
#endif
        int length = instruction(this);
        cycles += 2; // Every instruction usually takes at least 2 cycles.
        registers->programCounter += length;
        processNMI();
    }

    void CPU::exec() {
        while (!stopExecution) {
            step();
        }
    }
    void CPU::stopExec() {
        stopExecution = true;
    }

    CPU::CPU(ROM* rom) {
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
    }
}