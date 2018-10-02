//
// Created by Taylor Whatley on 2018-09-24.
//

#include "CPU.h"
#include "Operations.h"
#include "Codes.h"

namespace Nem {
    bool isNegative(Byte value) { return (value & 0b10000000) == 0b10000000; }

    void checkASLOverflow(CPU* cpu, Byte value) {
        cpu->setFlags(((int)value << 1) > 255, CPURegisters::StatusFlags::Carry);
    }

    void checkLSROverflow(CPU* cpu, Byte value) {
        cpu->setFlags((value & 0b00000001) == 0b00000001, CPURegisters::StatusFlags::Carry);
    }

    void checkZero(CPU *cpu, Byte value) {
        cpu->setFlags(value == 0, CPURegisters::StatusFlags::Zero);
    }

    void checkZero(CPU *cpu) {
        checkZero(cpu, cpu->registers->accumulator);
    }

    void checkNegative(CPU *cpu, Byte value) {
        cpu->setFlags(isNegative(value), CPURegisters::StatusFlags::Negative);
    }

    void checkNegative(CPU *cpu) {
        checkNegative(cpu, cpu->registers->accumulator);
    }

    void checkLow(CPU *cpu, Byte value) {
        checkZero(cpu, value);
        checkNegative(cpu, value);
    }

    void checkLow(CPU *cpu) {
        checkLow(cpu, cpu->registers->accumulator);
    }

    Address onPage(Address offset, Byte page) {
        return (Address) (offset % 0x100 + page * 0x100);
    }

    Byte getByteOnPage(CPU *cpu, Address offset, Byte page) {
        return cpu->memory->getByte(onPage(offset, page));
    }

    Address getAddressOnPage(CPU *cpu, Address offset, Byte page) {
        Byte a = cpu->memory->getByte(onPage(offset, page));
        Byte b = cpu->memory->getByte(onPage(offset + (Address) 1, page));
        return makeAddress(a, b);
    }

    void CMP(CPU *cpu, Byte reg, Byte value) {
        cpu->setFlags(reg >= value, CPURegisters::StatusFlags::Carry);
        bool equal = reg == value;
        cpu->setFlags(equal, CPURegisters::StatusFlags::Zero);
        if (equal) cpu->clearFlags(CPURegisters::StatusFlags::Negative);
        else checkNegative(cpu, reg - value);
    }

    void ADC(CPU *cpu, Byte value) {
        bool carry = cpu->isFlagSet(CPURegisters::StatusFlags::Carry);
        bool test = (int) cpu->registers->accumulator + (int) value + (int) carry > 255;
        cpu->setFlags((bool) (
                              ~(cpu->registers->accumulator ^ value) &
                              (cpu->registers->accumulator ^ (cpu->registers->accumulator + value + (Byte) carry)) & 0x80),
                      CPURegisters::StatusFlags::Overflow);
        cpu->registers->accumulator += value + (Byte) cpu->isFlagSet(CPURegisters::StatusFlags::Carry);
        cpu->setFlags(test, CPURegisters::StatusFlags::Carry);
        checkLow(cpu);
    }

    void SBC(CPU *cpu, Byte value) {
        bool carry = cpu->isFlagSet(CPURegisters::StatusFlags::Carry);
        bool test = cpu->registers->accumulator >= value + (Byte) !carry;
        cpu->setFlags(
                makeSigned(cpu->registers->accumulator) - makeSigned(value) > 127 ||
                makeSigned(cpu->registers->accumulator) - makeSigned(value) < -128,
                CPURegisters::StatusFlags::Overflow);
        cpu->registers->accumulator -= value + (Byte) !cpu->isFlagSet(CPURegisters::StatusFlags::Carry);
        cpu->setFlags(test, CPURegisters::StatusFlags::Carry);
        checkLow(cpu);
    }

    void INC(CPU *cpu, Address pointer) {
        cpu->memory->setByte(pointer, cpu->memory->getByte(pointer) + (Byte) 1);
        checkLow(cpu, cpu->memory->getByte(pointer));
    }

    void DEC(CPU *cpu, Address pointer) {
        cpu->memory->setByte(pointer, cpu->memory->getByte(pointer) - (Byte) 1);
        checkLow(cpu, cpu->memory->getByte(pointer));
    }

    void AND(CPU *cpu, Byte value) {
        cpu->registers->accumulator &= value;
        checkLow(cpu);
    }

    void ORA(CPU *cpu, Byte value) {
        cpu->registers->accumulator |= value;
        checkLow(cpu);
    }

    void EOR(CPU *cpu, Byte value) {
        cpu->registers->accumulator ^= value;
        checkLow(cpu);
    }

    void BIT(CPU *cpu, Byte value) {
        cpu->setFlags((cpu->registers->accumulator & value) == 0, CPURegisters::StatusFlags::Zero);
        cpu->setFlags((value & 0b01000000) > 0, CPURegisters::StatusFlags::Overflow);
        checkNegative(cpu, value);
    }

    void JMP(CPU* cpu, Address pointer) {
        cpu->registers->programCounter = pointer - (Address)1;
    }

    void BXS(CPU* cpu, CPURegisters::StatusFlags flags, Byte jump) {
        if (cpu->isFlagSet(flags)) {
            short value = makeSigned(jump);
            if (skippedPage(cpu->registers->programCounter, value)) cpu->cycles += 1;
            cpu->registers->programCounter += value;
            cpu->cycles += 1;
        }
    }
    void BXC(CPU* cpu, CPURegisters::StatusFlags flags, Byte jump) {
        if (!cpu->isFlagSet(flags)) {
            short value = makeSigned(jump);
            if (skippedPage(cpu->registers->programCounter, value)) cpu->cycles += 1;
            cpu->registers->programCounter += value;
            cpu->cycles += 1;
        }
    }

    void ASL(CPU* cpu) {
        checkASLOverflow(cpu, cpu->registers->accumulator);
        cpu->registers->accumulator = cpu->registers->accumulator << 1;
        checkLow(cpu);
    }

    void ASL(CPU* cpu, Address pointer) {
        Byte value = cpu->memory->getByte(pointer);
        checkASLOverflow(cpu, value);
        cpu->memory->setByte(pointer, value << 1);
        checkLow(cpu, value << 1);
    }

    void LSR(CPU* cpu) {
        checkLSROverflow(cpu, cpu->registers->accumulator);
        cpu->registers->accumulator = cpu->registers->accumulator >> 1;
        checkLow(cpu);
    }

    void LSR(CPU* cpu, Address pointer) {
        Byte value = cpu->memory->getByte(pointer);
        checkLSROverflow(cpu, value);
        cpu->memory->setByte(pointer, value >> 1);
        checkLow(cpu, value >> 1);
    }

    void ROL(CPU* cpu) {
        Byte bit0 = cpu->isFlagSet(CPURegisters::StatusFlags::Carry) ? (Byte)0b00000001 : (Byte)0b00000000;
        cpu->setFlags((cpu->registers->accumulator & 0b10000000) == 0b10000000, CPURegisters::StatusFlags::Carry);
        cpu->registers->accumulator = cpu->registers->accumulator << 1;
        cpu->registers->accumulator |= bit0;
        checkLow(cpu);
    }
    void ROL(CPU* cpu, Address pointer) {
        Byte bit0 = cpu->isFlagSet(CPURegisters::StatusFlags::Carry) ? (Byte)0b00000001 : (Byte)0b00000000;
        cpu->setFlags((cpu->memory->getByte(pointer) & 0b10000000) == 0b10000000,
                      CPURegisters::StatusFlags::Carry);
        cpu->memory->setByte(pointer, cpu->memory->getByte(pointer) << 1);
        cpu->memory->setByte(pointer, cpu->memory->getByte(pointer) | bit0);
        checkLow(cpu, cpu->memory->getByte(pointer));
    }
    void ROR(CPU* cpu) {
        Byte bit7 = cpu->isFlagSet(CPURegisters::StatusFlags::Carry) ? (Byte)0b10000000 : (Byte)0b00000000;
        cpu->setFlags((cpu->registers->accumulator & 0b00000001) == 0b00000001, CPURegisters::StatusFlags::Carry);
        cpu->registers->accumulator = cpu->registers->accumulator >> 1;
        cpu->registers->accumulator |= bit7;
        checkLow(cpu);
    }
    void ROR(CPU* cpu, Address pointer) {
        Byte bit7 = cpu->isFlagSet(CPURegisters::StatusFlags::Carry) ? (Byte)0b10000000 : (Byte)0b00000000;
        cpu->setFlags((cpu->memory->getByte(pointer) & 0b00000001) == 0b00000001, CPURegisters::StatusFlags::Carry);
        cpu->memory->setByte(pointer, cpu->memory->getByte(pointer) >> 1);
        cpu->memory->setByte(pointer, cpu->memory->getByte(pointer) | bit7);
        checkLow(cpu, cpu->memory->getByte(pointer));
    }
}