//
// Created by Taylor Whatley on 2018-09-19.
//

#include "CPUCodes.h"

#include "CPU.h"

#ifdef NEM_PROFILE
#include "../Debug/Profiler.h"
#endif

#include <functional>
#include <iostream>

namespace Nem {
    bool isNegative(Byte value) {
        return (value & 0b10000000) == 0b10000000;
    }

    bool skippedPage(Address pointer, short offset) {
        return pointer / 256 < (pointer + offset) / 256;
    }

    bool skippedPage(Address pointer, Byte offset) {
        return skippedPage(pointer, (short) offset);
    }

    void checkZero(CPU *cpu, Byte value) {
        cpu->setFlags(value == 0, CPURegisters::StatusFlags::Zero);
    }

    void checkNegative(CPU *cpu, Byte value) {
        cpu->setFlags(isNegative(value), CPURegisters::StatusFlags::Negative);
    }

    void checkLow(CPU *cpu, Byte value) {
        checkZero(cpu, value);
        checkNegative(cpu, value);
    }

    void checkLow(CPU *cpu) {
        checkLow(cpu, cpu->registers->accumulator);
    }

    Address onPage(Address offset, Byte page = 0) {
        return (Address)(offset % 0x100 + page * 0x100);
    }

    Byte getByteOnPage(CPU *cpu, Address offset, Byte page = 0) {
        return cpu->memory->getByte(onPage(offset, page));
    }

    Address getAddressOnPage(CPU *cpu, Address offset, Byte page = 0) {
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
        cpu->setFlags(
                (bool) (
                        ~(cpu->registers->accumulator ^ value) &
                        (cpu->registers->accumulator ^ (cpu->registers->accumulator + value + (Byte) carry)) & 0x80),
                CPURegisters::StatusFlags::Overflow);
        cpu->registers->accumulator += value + (Byte) cpu->isFlagSet(CPURegisters::StatusFlags::Carry);
        cpu->setFlags(test, CPURegisters::StatusFlags::Carry);
        checkLow(cpu);
    }

    void SBC(CPU *cpu, Byte value) { ADC(cpu, ~value); }

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

    void JMP(CPU *cpu, Address pointer) {
        cpu->registers->programCounter = pointer - (Address) 1;
    }

    void BXS(CPU *cpu, CPURegisters::StatusFlags flags, Byte jump) {
        if (cpu->isFlagSet(flags)) {
            short value = makeSigned(jump);
            if (skippedPage(cpu->registers->programCounter, value)) cpu->cycles += 1;
            cpu->registers->programCounter += value;
            cpu->cycles += 1;
        }
    }

    void BXC(CPU *cpu, CPURegisters::StatusFlags flags, Byte jump) {
        if (!cpu->isFlagSet(flags)) {
            short value = makeSigned(jump);
            if (skippedPage(cpu->registers->programCounter, value)) cpu->cycles += 1;
            cpu->registers->programCounter += value;
            cpu->cycles += 1;
        }
    }

    void ASL(CPU *cpu) {
        cpu->setFlags(((int) cpu->registers->accumulator << 1) > 255, CPURegisters::StatusFlags::Carry);
        cpu->registers->accumulator = cpu->registers->accumulator << 1;
        checkLow(cpu);
    }

    void ASL(CPU *cpu, Address pointer) {
        Byte value = cpu->memory->getByte(pointer);
        cpu->setFlags(((int) value << 1) > 255, CPURegisters::StatusFlags::Carry);
        cpu->memory->setByte(pointer, value << 1);
        checkLow(cpu, value << 1);
    }

    void LSR(CPU *cpu) {
        cpu->setFlags((cpu->registers->accumulator & 0b00000001) == 0b00000001, CPURegisters::StatusFlags::Carry);
        cpu->registers->accumulator = cpu->registers->accumulator >> 1;
        checkLow(cpu);
    }

    void LSR(CPU *cpu, Address pointer) {
        Byte value = cpu->memory->getByte(pointer);
        cpu->setFlags((value & 0b00000001) == 0b00000001, CPURegisters::StatusFlags::Carry);
        cpu->memory->setByte(pointer, value >> 1);
        checkLow(cpu, value >> 1);
    }

    void ROL(CPU *cpu) {
        Byte bit0 = cpu->isFlagSet(CPURegisters::StatusFlags::Carry) ? (Byte) 0b00000001 : (Byte) 0b00000000;
        cpu->setFlags((cpu->registers->accumulator & 0b10000000) == 0b10000000, CPURegisters::StatusFlags::Carry);
        cpu->registers->accumulator = cpu->registers->accumulator << 1;
        cpu->registers->accumulator |= bit0;
        checkLow(cpu);
    }

    void ROL(CPU *cpu, Address pointer) {
        Byte bit0 = cpu->isFlagSet(CPURegisters::StatusFlags::Carry) ? (Byte) 0b00000001 : (Byte) 0b00000000;
        cpu->setFlags((cpu->memory->getByte(pointer) & 0b10000000) == 0b10000000,
                      CPURegisters::StatusFlags::Carry);
        cpu->memory->setByte(pointer, cpu->memory->getByte(pointer) << 1);
        cpu->memory->setByte(pointer, cpu->memory->getByte(pointer) | bit0);
        checkLow(cpu, cpu->memory->getByte(pointer));
    }

    void ROR(CPU *cpu) {
        Byte bit7 = cpu->isFlagSet(CPURegisters::StatusFlags::Carry) ? (Byte) 0b10000000 : (Byte) 0b00000000;
        cpu->setFlags((cpu->registers->accumulator & 0b00000001) == 0b00000001, CPURegisters::StatusFlags::Carry);
        cpu->registers->accumulator = cpu->registers->accumulator >> 1;
        cpu->registers->accumulator |= bit7;
        checkLow(cpu);
    }

    void ROR(CPU *cpu, Address pointer) {
        Byte bit7 = cpu->isFlagSet(CPURegisters::StatusFlags::Carry) ? (Byte) 0b10000000 : (Byte) 0b00000000;
        cpu->setFlags((cpu->memory->getByte(pointer) & 0b00000001) == 0b00000001, CPURegisters::StatusFlags::Carry);
        cpu->memory->setByte(pointer, cpu->memory->getByte(pointer) >> 1);
        cpu->memory->setByte(pointer, cpu->memory->getByte(pointer) | bit7);
        checkLow(cpu, cpu->memory->getByte(pointer));
    }

    int Unimplemented(CPU *cpu) {
        std::cout << "Instruction not implemented. PC: $" << makeHex(cpu->registers->programCounter)
                  << " INST: $" << makeHex(cpu->memory->getByte(cpu->registers->programCounter)) << std::endl;

#ifdef NEM_PROFILE
        cpu->profiler->breakpoint();
#endif

        cpu->stopExec();

        return 0;
    }

    int NOPInstruction(CPU*) { return 0; }
    int NOPInstruction_i(CPU*) { return 1; }
    int NOPInstruction_d(CPU *cpu) { cpu->cycles += 1; return 1; }
    int NOPInstruction_d_x(CPU *cpu) { cpu->cycles += 2; return 1; }
    int NOPInstruction_a(CPU *cpu) { cpu->cycles += 2; return 2; }
    int NOPInstruction_a_x(CPU *cpu) {
        cpu->cycles += 2;
        if (skippedPage(cpu->nextAddress(), cpu->registers->indexX)) cpu->cycles += 1;
        return 2;
    }

    int ADCInstruction_i(CPU *cpu) {
        Byte value = cpu->nextByte();
        ADC(cpu, value);
        return 1;
    }

    int ADCInstruction_d(CPU *cpu) {
        Byte value = cpu->memory->getByte((Address) cpu->nextByte());
        ADC(cpu, value);
        cpu->cycles += 1;
        return 1;
    }

    int ADCInstruction_d_x(CPU *cpu) {
        Byte value = getByteOnPage(cpu, (Address) (cpu->nextByte() + cpu->registers->indexX));
        ADC(cpu, value);
        cpu->cycles += 2;
        return 1;
    }

    int ADCInstruction_a(CPU *cpu) {
        Byte value = cpu->memory->getByte(cpu->nextAddress());
        ADC(cpu, value);
        cpu->cycles += 2;
        return 2;
    }

    int ADCInstruction_a_x(CPU *cpu) {
        Address pointer = cpu->nextAddress();
        if (skippedPage(pointer, cpu->registers->indexX)) cpu->cycles += 1;
        Byte value = cpu->memory->getByte(pointer + cpu->registers->indexX);
        ADC(cpu, value);
        cpu->cycles += 2;
        return 2;
    }

    int ADCInstruction_a_y(CPU *cpu) {
        Address pointer = cpu->nextAddress();
        if (skippedPage(pointer, cpu->registers->indexY)) cpu->cycles += 1;
        Byte value = cpu->memory->getByte(pointer + cpu->registers->indexY);
        ADC(cpu, value);
        cpu->cycles += 2;
        return 2;
    }

    int ADCInstruction_$x(CPU *cpu) {
        Address pointer = getAddressOnPage(cpu, (Address) (cpu->nextByte() + cpu->registers->indexX));
        Byte value = cpu->memory->getByte(pointer);
        ADC(cpu, value);
        cpu->cycles += 4;
        return 1;
    }

    int ADCInstruction_$y(CPU *cpu) {
        Address pointer = getAddressOnPage(cpu, (Address) cpu->nextByte());
        if (skippedPage(pointer, cpu->registers->indexY)) cpu->cycles += 1;
        Byte value = cpu->memory->getByte(pointer + cpu->registers->indexY);
        ADC(cpu, value);
        cpu->cycles += 3;
        return 1;
    }

    int SBCInstruction_i(CPU *cpu) {
        Byte value = cpu->nextByte();
        SBC(cpu, value);
        return 1;
    }

    int SBCInstruction_d(CPU *cpu) {
        Byte value = cpu->memory->getByte((Address) cpu->nextByte());
        SBC(cpu, value);
        cpu->cycles += 1;
        return 1;
    }

    int SBCInstruction_d_x(CPU *cpu) {
        Byte value = getByteOnPage(cpu, (Address) cpu->nextByte() + cpu->registers->indexX);
        SBC(cpu, value);
        cpu->cycles += 2;
        return 1;
    }

    int SBCInstruction_a(CPU *cpu) {
        Byte value = cpu->memory->getByte(cpu->nextAddress());
        SBC(cpu, value);
        cpu->cycles += 2;
        return 2;
    }

    int SBCInstruction_a_x(CPU *cpu) {
        Address pointer = cpu->nextAddress();
        if (skippedPage(pointer, cpu->registers->indexX)) cpu->cycles += 1;
        Byte value = cpu->memory->getByte(pointer + cpu->registers->indexX);
        SBC(cpu, value);
        cpu->cycles += 2;
        return 2;
    }

    int SBCInstruction_a_y(CPU *cpu) {
        Address pointer = cpu->nextAddress();
        if (skippedPage(pointer, cpu->registers->indexY)) cpu->cycles += 1;
        Byte value = cpu->memory->getByte(pointer + cpu->registers->indexY);
        SBC(cpu, value);
        cpu->cycles += 2;
        return 2;
    }

    int SBCInstruction_$x(CPU *cpu) {
        Address pointer = getAddressOnPage(cpu, (Address) cpu->nextByte() + cpu->registers->indexX);
        Byte value = cpu->memory->getByte(pointer);
        SBC(cpu, value);
        cpu->cycles += 4;
        return 1;
    }

    int SBCInstruction_$y(CPU *cpu) {
        Address pointer = getAddressOnPage(cpu, (Address) cpu->nextByte());
        if (skippedPage(pointer, cpu->registers->indexY)) cpu->cycles += 1;
        Byte value = cpu->memory->getByte(pointer + cpu->registers->indexY);
        SBC(cpu, value);
        cpu->cycles += 3;
        return 1;
    }

    int INCInstruction_d(CPU *cpu) {
        Address pointer = (Address) cpu->nextByte();
        INC(cpu, pointer);
        cpu->cycles += 3;
        return 1;
    }

    int INCInstruction_d_x(CPU *cpu) {
        Address pointer = onPage((Address) cpu->nextByte() + cpu->registers->indexX);
        INC(cpu, pointer);
        cpu->cycles += 4;
        return 1;
    }

    int INCInstruction_a(CPU *cpu) {
        Address pointer = cpu->nextAddress();
        INC(cpu, pointer);
        cpu->cycles += 4;
        return 2;
    }

    int INCInstruction_a_x(CPU *cpu) {
        Address pointer = cpu->nextAddress() + cpu->registers->indexX;
        INC(cpu, pointer);
        cpu->cycles += 5;
        return 2;
    }

    int DECInstruction_d(CPU *cpu) {
        Address pointer = (Address) cpu->nextByte();
        DEC(cpu, pointer);
        cpu->cycles += 3;
        return 1;
    }

    int DECInstruction_d_x(CPU *cpu) {
        Address pointer = onPage((Address)cpu->nextByte() + cpu->registers->indexX);
        DEC(cpu, pointer);
        cpu->cycles += 4;
        return 1;
    }

    int DECInstruction_a(CPU *cpu) {
        Address pointer = cpu->nextAddress();
        DEC(cpu, pointer);
        cpu->cycles += 4;
        return 2;
    }

    int DECInstruction_a_x(CPU *cpu) {
        Address pointer = cpu->nextAddress() + cpu->registers->indexX;
        DEC(cpu, pointer);
        cpu->cycles += 5;
        return 2;
    }

    int ISBInstruction_d(CPU *cpu) {
        Address pointer = (Address) cpu->nextByte();
        INC(cpu, pointer);
        SBC(cpu, cpu->memory->getByte(pointer));
        cpu->cycles += 3;
        return 1;
    }

    int ISBInstruction_d_x(CPU *cpu) {
        Address pointer = onPage((Address) cpu->nextByte() + cpu->registers->indexX);
        INC(cpu, pointer);
        SBC(cpu, cpu->memory->getByte(pointer));
        cpu->cycles += 4;
        return 1;
    }

    int ISBInstruction_a(CPU *cpu) {
        Address pointer = (Address) cpu->nextAddress();
        INC(cpu, pointer);
        SBC(cpu, cpu->memory->getByte(pointer));
        cpu->cycles += 4;
        return 2;
    }

    int ISBInstruction_a_x(CPU *cpu) {
        Address pointer = (Address) cpu->nextAddress() + cpu->registers->indexX;
        INC(cpu, pointer);
        SBC(cpu, cpu->memory->getByte(pointer));
        cpu->cycles += 5;
        return 2;
    }

    int ISBInstruction_a_y(CPU *cpu) {
        Address pointer = (Address) cpu->nextAddress() + cpu->registers->indexY;
        INC(cpu, pointer);
        SBC(cpu, cpu->memory->getByte(pointer));
        cpu->cycles += 5;
        return 2;
    }

    int ISBInstruction_$x(CPU *cpu) {
        Address pointer = getAddressOnPage(cpu, (Address) cpu->nextAddress() + cpu->registers->indexX);
        INC(cpu, pointer);
        SBC(cpu, cpu->memory->getByte(pointer));
        cpu->cycles += 6;
        return 1;
    }

    int ISBInstruction_$y(CPU *cpu) {
        Address pointer = getAddressOnPage(cpu, (Address) cpu->nextAddress()) + cpu->registers->indexY;
        INC(cpu, pointer);
        SBC(cpu, cpu->memory->getByte(pointer));
        cpu->cycles += 6;
        return 1;
    }

    int DCPInstruction_d(CPU *cpu) {
        Address pointer = (Address) cpu->nextByte();
        DEC(cpu, pointer);
        CMP(cpu, cpu->registers->accumulator, cpu->memory->getByte(pointer));
        cpu->cycles += 3;
        return 1;
    }

    int DCPInstruction_d_x(CPU *cpu) {
        Address pointer = onPage((Address) cpu->nextByte() + cpu->registers->indexX);
        DEC(cpu, pointer);
        CMP(cpu, cpu->registers->accumulator, cpu->memory->getByte(pointer));
        cpu->cycles += 4;
        return 1;
    }

    int DCPInstruction_a(CPU *cpu) {
        Address pointer = cpu->nextAddress();
        DEC(cpu, pointer);
        CMP(cpu, cpu->registers->accumulator, cpu->memory->getByte(pointer));
        cpu->cycles += 4;
        return 2;
    }

    int DCPInstruction_a_x(CPU *cpu) {
        Address pointer = cpu->nextAddress() + cpu->registers->indexX;
        DEC(cpu, pointer);
        CMP(cpu, cpu->registers->accumulator, cpu->memory->getByte(pointer));
        cpu->cycles += 5;
        return 2;
    }

    int DCPInstruction_a_y(CPU *cpu) {
        Address pointer = cpu->nextAddress() + cpu->registers->indexY;
        DEC(cpu, pointer);
        CMP(cpu, cpu->registers->accumulator, cpu->memory->getByte(pointer));
        cpu->cycles += 5;
        return 2;
    }

    int DCPInstruction_$x(CPU *cpu) {
        Address pointer = getAddressOnPage(cpu, (Address) cpu->nextByte() + cpu->registers->indexX);
        DEC(cpu, pointer);
        CMP(cpu, cpu->registers->accumulator, cpu->memory->getByte(pointer));
        cpu->cycles += 6;
        return 1;
    }

    int DCPInstruction_$y(CPU *cpu) {
        Address pointer = getAddressOnPage(cpu, (Address) cpu->nextByte()) + cpu->registers->indexY;
        DEC(cpu, pointer);
        CMP(cpu, cpu->registers->accumulator, cpu->memory->getByte(pointer));
        cpu->cycles += 6;
        return 1;
    }

    int ANDInstruction_i(CPU *cpu) {
        AND(cpu, cpu->nextByte());
        return 1;
    }

    int ANDInstruction_d(CPU *cpu) {
        AND(cpu, cpu->memory->getByte((Address) cpu->nextByte()));
        cpu->cycles += 1;
        return 1;
    }

    int ANDInstruction_d_x(CPU *cpu) {
        AND(cpu, getByteOnPage(cpu, (Address) (cpu->nextByte() + cpu->registers->indexX)));
        cpu->cycles += 2;
        return 1;
    }

    int ANDInstruction_a(CPU *cpu) {
        AND(cpu, cpu->memory->getByte(cpu->nextAddress()));
        cpu->cycles += 2;
        return 2;
    }

    int ANDInstruction_a_x(CPU *cpu) {
        Address pointer = cpu->nextAddress();
        if (skippedPage(pointer, cpu->registers->indexX)) cpu->cycles += 1;
        AND(cpu, cpu->memory->getByte(pointer + cpu->registers->indexX));
        cpu->cycles += 2;
        return 2;
    }

    int ANDInstruction_a_y(CPU *cpu) {
        Address pointer = cpu->nextAddress();
        if (skippedPage(pointer, cpu->registers->indexY)) cpu->cycles += 1;
        AND(cpu, cpu->memory->getByte(pointer + cpu->registers->indexY));
        cpu->cycles += 2;
        return 2;
    }

    int ANDInstruction_$x(CPU *cpu) {
        Address pointer = getAddressOnPage(cpu, (Address) (cpu->nextByte() + cpu->registers->indexX));
        AND(cpu, cpu->memory->getByte(pointer));
        cpu->cycles += 4;
        return 1;
    }

    int ANDInstruction_$y(CPU *cpu) {
        Address pointer = getAddressOnPage(cpu, (Address) cpu->nextByte());
        if (skippedPage(pointer, cpu->registers->indexY)) cpu->cycles += 1;
        AND(cpu, cpu->memory->getByte(pointer + cpu->registers->indexY));
        cpu->cycles += 3;
        return 1;
    }

    int ORAInstruction_i(CPU *cpu) {
        ORA(cpu, cpu->nextByte());
        return 1;
    }

    int ORAInstruction_d(CPU *cpu) {
        ORA(cpu, cpu->memory->getByte((Address) cpu->nextByte()));
        cpu->cycles += 1;
        return 1;
    }

    int ORAInstruction_d_x(CPU *cpu) {
        ORA(cpu, getByteOnPage(cpu, (Address) cpu->nextByte() + cpu->registers->indexX));
        cpu->cycles += 2;
        return 1;
    }

    int ORAInstruction_a(CPU *cpu) {
        ORA(cpu, cpu->memory->getByte(cpu->nextAddress()));
        cpu->cycles += 2;
        return 2;
    }

    int ORAInstruction_a_x(CPU *cpu) {
        Address pointer = cpu->nextAddress();
        if (skippedPage(pointer, cpu->registers->indexX)) cpu->cycles += 1;
        ORA(cpu, cpu->memory->getByte(pointer + cpu->registers->indexX));
        cpu->cycles += 2;
        return 2;
    }

    int ORAInstruction_a_y(CPU *cpu) {
        Address pointer = cpu->nextAddress();
        if (skippedPage(pointer, cpu->registers->indexX)) cpu->cycles += 1;
        ORA(cpu, cpu->memory->getByte(pointer + cpu->registers->indexY));
        cpu->cycles += 2;
        return 2;
    }

    int ORAInstruction_$x(CPU *cpu) {
        Address pointer = getAddressOnPage(cpu, (Address) cpu->nextByte() + cpu->registers->indexX);
        ORA(cpu, cpu->memory->getByte(pointer));
        cpu->cycles += 4;
        return 1;
    }

    int ORAInstruction_$y(CPU *cpu) {
        Address pointer = getAddressOnPage(cpu, (Address) cpu->nextByte());
        if (skippedPage(pointer, cpu->registers->indexY)) cpu->cycles += 1;
        ORA(cpu, cpu->memory->getByte(pointer + cpu->registers->indexY));
        cpu->cycles += 3;
        return 1;
    }

    int EORInstruction_i(CPU *cpu) {
        EOR(cpu, cpu->nextByte());
        return 1;
    }

    int EORInstruction_d(CPU *cpu) {
        EOR(cpu, cpu->memory->getByte((Address) cpu->nextByte()));
        cpu->cycles += 1;
        return 1;
    }

    int EORInstruction_d_x(CPU *cpu) {
        EOR(cpu, getByteOnPage(cpu, (Address) cpu->nextByte() + cpu->registers->indexX));
        cpu->cycles += 2;
        return 1;
    }

    int EORInstruction_a(CPU *cpu) {
        EOR(cpu, cpu->memory->getByte(cpu->nextAddress()));
        cpu->cycles += 2;
        return 2;
    }

    int EORInstruction_a_x(CPU *cpu) {
        Address pointer = cpu->nextAddress();
        if (skippedPage(pointer, cpu->registers->indexX)) cpu->cycles += 1;
        EOR(cpu, cpu->memory->getByte(pointer + cpu->registers->indexX));
        cpu->cycles += 2;
        return 2;
    }

    int EORInstruction_a_y(CPU *cpu) {
        Address pointer = cpu->nextAddress();
        if (skippedPage(pointer, cpu->registers->indexY)) cpu->cycles += 1;
        EOR(cpu, cpu->memory->getByte(pointer + cpu->registers->indexY));
        cpu->cycles += 2;
        return 2;
    }

    int EORInstruction_$x(CPU *cpu) {
        Address pointer = getAddressOnPage(cpu, (Address) cpu->nextByte() + cpu->registers->indexX);
        EOR(cpu, cpu->memory->getByte(pointer));
        cpu->cycles += 4;
        return 1;
    }

    int EORInstruction_$y(CPU *cpu) {
        Address pointer = getAddressOnPage(cpu, (Address) cpu->nextByte());
        if (skippedPage(pointer, cpu->registers->indexY)) cpu->cycles += 1;
        EOR(cpu, cpu->memory->getByte(pointer + cpu->registers->indexY));
        cpu->cycles += 3;
        return 1;
    }

    int BITInstruction_d(CPU *cpu) {
        BIT(cpu, cpu->memory->getByte((Address) cpu->nextByte()));
        cpu->cycles += 1;
        return 1;
    }

    int BITInstruction_a(CPU *cpu) {
        BIT(cpu, cpu->memory->getByte(cpu->nextAddress()));
        cpu->cycles += 2;
        return 2;
    }

    int CMPInstruction_i(CPU *cpu) {
        Byte value = cpu->nextByte();
        CMP(cpu, cpu->registers->accumulator, value);
        return 1;
    }

    int CMPInstruction_d(CPU *cpu) {
        Byte value = cpu->memory->getByte((Address) cpu->nextByte());
        CMP(cpu, cpu->registers->accumulator, value);
        cpu->cycles += 1;
        return 1;
    }

    int CMPInstruction_d_x(CPU *cpu) {
        Byte value = getByteOnPage(cpu, (Address) cpu->nextByte() + cpu->registers->indexX);
        CMP(cpu, cpu->registers->accumulator, value);
        cpu->cycles += 2;
        return 1;
    }

    int CMPInstruction_a(CPU *cpu) {
        Byte value = cpu->memory->getByte(cpu->nextAddress());
        CMP(cpu, cpu->registers->accumulator, value);
        cpu->cycles += 2;
        return 2;
    }

    int CMPInstruction_a_x(CPU *cpu) {
        Address pointer = cpu->nextAddress();
        if (skippedPage(pointer, cpu->registers->indexX)) cpu->cycles += 1;
        Byte value = cpu->memory->getByte(pointer + cpu->registers->indexX);
        CMP(cpu, cpu->registers->accumulator, value);
        cpu->cycles += 2;
        return 2;
    }

    int CMPInstruction_a_y(CPU *cpu) {
        Address pointer = cpu->nextAddress();
        if (skippedPage(pointer, cpu->registers->indexY)) cpu->cycles += 1;
        Byte value = cpu->memory->getByte(pointer + cpu->registers->indexY);
        CMP(cpu, cpu->registers->accumulator, value);
        cpu->cycles += 2;
        return 2;
    }

    int CMPInstruction_$x(CPU *cpu) {
        Byte value = cpu->memory->getByte(getAddressOnPage(cpu, cpu->nextByte() + cpu->registers->indexX));
        CMP(cpu, cpu->registers->accumulator, value);
        cpu->cycles += 4;
        return 1;
    }

    int CMPInstruction_$y(CPU *cpu) {
        Address pointer = getAddressOnPage(cpu, cpu->nextByte());
        if (skippedPage(pointer, cpu->registers->indexY)) cpu->cycles += 1;
        Byte value = cpu->memory->getByte(pointer + cpu->registers->indexY);
        CMP(cpu, cpu->registers->accumulator, value);
        cpu->cycles += 3;
        return 1;
    }

    int CPXInstruction_i(CPU *cpu) {
        Byte value = cpu->nextByte();
        CMP(cpu, cpu->registers->indexX, value);
        return 1;
    }

    int CPXInstruction_d(CPU *cpu) {
        Byte value = cpu->memory->getByte((Address) cpu->nextByte());
        CMP(cpu, cpu->registers->indexX, value);
        cpu->cycles += 1;
        return 1;
    }

    int CPXInstruction_a(CPU *cpu) {
        Byte value = cpu->memory->getByte(cpu->nextAddress());
        CMP(cpu, cpu->registers->indexX, value);
        cpu->cycles += 2;
        return 2;
    }

    int CPYInstruction_i(CPU *cpu) {
        Byte value = cpu->nextByte();
        CMP(cpu, cpu->registers->indexY, value);
        return 1;
    }

    int CPYInstruction_d(CPU *cpu) {
        Byte value = cpu->memory->getByte((Address) cpu->nextByte());
        CMP(cpu, cpu->registers->indexY, value);
        cpu->cycles += 1;
        return 1;
    }

    int CPYInstruction_a(CPU *cpu) {
        Byte value = cpu->memory->getByte(cpu->nextAddress());
        CMP(cpu, cpu->registers->indexY, value);
        cpu->cycles += 2;
        return 2;
    }

    int ASLInstruction(CPU *cpu) {
        ASL(cpu);
        return 0;
    }

    int ASLInstruction_d(CPU *cpu) {
        ASL(cpu, (Address) cpu->nextByte());
        cpu->cycles += 3;
        return 1;
    }

    int ASLInstruction_d_x(CPU *cpu) {
        ASL(cpu, onPage((Address) (cpu->nextByte() + cpu->registers->indexX)));
        cpu->cycles += 4;
        return 1;
    }

    int ASLInstruction_a(CPU *cpu) {
        ASL(cpu, cpu->nextAddress());
        cpu->cycles += 4;
        return 2;
    }

    int ASLInstruction_a_x(CPU *cpu) {
        ASL(cpu, cpu->nextAddress() + cpu->registers->indexX);
        cpu->cycles += 5;
        return 2;
    }

    int LSRInstruction(CPU *cpu) {
        LSR(cpu);
        return 0;
    }

    int LSRInstruction_d(CPU *cpu) {
        LSR(cpu, (Address) cpu->nextByte());
        cpu->cycles += 3;
        return 1;
    }

    int LSRInstruction_d_x(CPU *cpu) {
        LSR(cpu, onPage((Address) (cpu->nextByte() + cpu->registers->indexX)));
        cpu->cycles += 4;
        return 1;
    }

    int LSRInstruction_a(CPU *cpu) {
        LSR(cpu, cpu->nextAddress());
        cpu->cycles += 4;
        return 2;
    }

    int LSRInstruction_a_x(CPU *cpu) {
        LSR(cpu, cpu->nextAddress() + cpu->registers->indexX);
        cpu->cycles += 5;
        return 2;
    }

    int ROLInstruction(CPU *cpu) {
        ROL(cpu);
        return 0;
    }

    int ROLInstruction_d(CPU *cpu) {
        Address pointer = (Address) cpu->nextByte();
        ROL(cpu, pointer);
        cpu->cycles += 3;
        return 1;
    }

    int ROLInstruction_d_x(CPU *cpu) {
        Address pointer = onPage((Address)cpu->nextByte() + cpu->registers->indexX);
        ROL(cpu, pointer);
        cpu->cycles += 4;
        return 1;
    }

    int ROLInstruction_a(CPU *cpu) {
        Address pointer = cpu->nextAddress();
        ROL(cpu, pointer);
        cpu->cycles += 4;
        return 2;
    }

    int ROLInstruction_a_x(CPU *cpu) {
        Address pointer = cpu->nextAddress() + cpu->registers->indexX;
        ROL(cpu, pointer);
        cpu->cycles += 5;
        return 2;
    }

    int RORInstruction(CPU *cpu) {
        ROR(cpu);
        return 0;
    }

    int RORInstruction_d(CPU *cpu) {
        Address pointer = (Address) cpu->nextByte();
        ROR(cpu, pointer);
        cpu->cycles += 3;
        return 1;
    }

    int RORInstruction_d_x(CPU *cpu) {
        Address pointer = onPage((Address)cpu->nextByte() + cpu->registers->indexX);
        ROR(cpu, pointer);
        cpu->cycles += 4;
        return 1;
    }

    int RORInstruction_a(CPU *cpu) {
        Address pointer = cpu->nextAddress();
        ROR(cpu, pointer);
        cpu->cycles += 4;
        return 2;
    }

    int RORInstruction_a_x(CPU *cpu) {
        Address pointer = cpu->nextAddress() + cpu->registers->indexX;
        ROR(cpu, pointer);
        cpu->cycles += 5;
        return 2;
    }

    int SLOInstruction_d(CPU *cpu) {
        Address pointer = (Address) cpu->nextByte();
        ASL(cpu, pointer);
        ORA(cpu, cpu->memory->getByte(pointer));
        cpu->cycles += 3;
        return 1;
    }

    int SLOInstruction_d_x(CPU *cpu) {
        Address pointer = onPage((Address) cpu->nextByte() + cpu->registers->indexX);
        ASL(cpu, pointer);
        ORA(cpu, cpu->memory->getByte(pointer));
        cpu->cycles += 4;
        return 1;
    }

    int SLOInstruction_a(CPU *cpu) {
        Address pointer = cpu->nextAddress();
        ASL(cpu, pointer);
        ORA(cpu, cpu->memory->getByte(pointer));
        cpu->cycles += 4;
        return 2;
    }

    int SLOInstruction_a_x(CPU *cpu) {
        Address pointer = cpu->nextAddress() + cpu->registers->indexX;
        ASL(cpu, pointer);
        ORA(cpu, cpu->memory->getByte(pointer));
        cpu->cycles += 5;
        return 2;
    }

    int SLOInstruction_a_y(CPU *cpu) {
        Address pointer = cpu->nextAddress() + cpu->registers->indexY;
        ASL(cpu, pointer);
        ORA(cpu, cpu->memory->getByte(pointer));
        cpu->cycles += 5;
        return 2;
    }

    int SLOInstruction_$x(CPU *cpu) {
        Address pointer = getAddressOnPage(cpu, (Address) cpu->nextByte() + cpu->registers->indexX);
        ASL(cpu, pointer);
        ORA(cpu, cpu->memory->getByte(pointer));
        cpu->cycles += 6;
        return 1;
    }

    int SLOInstruction_$y(CPU *cpu) {
        Address pointer = getAddressOnPage(cpu, (Address) cpu->nextByte()) + cpu->registers->indexY;
        ASL(cpu, pointer);
        ORA(cpu, cpu->memory->getByte(pointer));
        cpu->cycles += 6;
        return 1;
    }

    int RLAInstruction_d(CPU *cpu) {
        Address pointer = (Address) cpu->nextByte();
        ROL(cpu, pointer);
        AND(cpu, cpu->memory->getByte(pointer));
        cpu->cycles += 3;
        return 1;
    }

    int RLAInstruction_d_x(CPU *cpu) {
        Address pointer = onPage((Address) cpu->nextByte() + cpu->registers->indexX);
        ROL(cpu, pointer);
        AND(cpu, cpu->memory->getByte(pointer));
        cpu->cycles += 4;
        return 1;
    }

    int RLAInstruction_a(CPU *cpu) {
        Address pointer = cpu->nextAddress();
        ROL(cpu, pointer);
        AND(cpu, cpu->memory->getByte(pointer));
        cpu->cycles += 4;
        return 2;
    }

    int RLAInstruction_a_x(CPU *cpu) {
        Address pointer = cpu->nextAddress() + cpu->registers->indexX;
        ROL(cpu, pointer);
        AND(cpu, cpu->memory->getByte(pointer));
        cpu->cycles += 5;
        return 2;
    }

    int RLAInstruction_a_y(CPU *cpu) {
        Address pointer = cpu->nextAddress() + cpu->registers->indexY;
        ROL(cpu, pointer);
        AND(cpu, cpu->memory->getByte(pointer));
        cpu->cycles += 5;
        return 2;
    }

    int RLAInstruction_$x(CPU *cpu) {
        Address pointer = getAddressOnPage(cpu, (Address) cpu->nextByte() + cpu->registers->indexX);
        ROL(cpu, pointer);
        AND(cpu, cpu->memory->getByte(pointer));
        cpu->cycles += 6;
        return 1;
    }

    int RLAInstruction_$y(CPU *cpu) {
        Address pointer = getAddressOnPage(cpu, (Address) cpu->nextByte()) + cpu->registers->indexY;
        ROL(cpu, pointer);
        AND(cpu, cpu->memory->getByte(pointer));
        cpu->cycles += 6;
        return 1;
    }

    int RRAInstruction_d(CPU *cpu) {
        Address pointer = (Address) cpu->nextByte();
        ROR(cpu, pointer);
        ADC(cpu, cpu->memory->getByte(pointer));
        cpu->cycles += 3;
        return 1;
    }

    int RRAInstruction_d_x(CPU *cpu) {
        Address pointer = onPage((Address) cpu->nextByte() + cpu->registers->indexX);
        ROR(cpu, pointer);
        ADC(cpu, cpu->memory->getByte(pointer));
        cpu->cycles += 4;
        return 1;
    }

    int RRAInstruction_a(CPU *cpu) {
        Address pointer = cpu->nextAddress();
        ROR(cpu, pointer);
        ADC(cpu, cpu->memory->getByte(pointer));
        cpu->cycles += 4;
        return 2;
    }

    int RRAInstruction_a_x(CPU *cpu) {
        Address pointer = cpu->nextAddress() + cpu->registers->indexX;
        ROR(cpu, pointer);
        ADC(cpu, cpu->memory->getByte(pointer));
        cpu->cycles += 5;
        return 2;
    }

    int RRAInstruction_a_y(CPU *cpu) {
        Address pointer = cpu->nextAddress() + cpu->registers->indexY;
        ROR(cpu, pointer);
        ADC(cpu, cpu->memory->getByte(pointer));
        cpu->cycles += 5;
        return 2;
    }

    int RRAInstruction_$x(CPU *cpu) {
        Address pointer = getAddressOnPage(cpu, (Address) cpu->nextByte() + cpu->registers->indexX);
        ROR(cpu, pointer);
        ADC(cpu, cpu->memory->getByte(pointer));
        cpu->cycles += 6;
        return 1;
    }

    int RRAInstruction_$y(CPU *cpu) {
        Address pointer = getAddressOnPage(cpu, (Address) cpu->nextByte()) + cpu->registers->indexY;
        ROR(cpu, pointer);
        ADC(cpu, cpu->memory->getByte(pointer));
        cpu->cycles += 6;
        return 1;
    }

    int SREInstruction_d(CPU *cpu) {
        Address pointer = (Address) cpu->nextByte();
        LSR(cpu, pointer);
        EOR(cpu, cpu->memory->getByte(pointer));
        cpu->cycles += 3;
        return 1;
    }

    int SREInstruction_d_x(CPU *cpu) {
        Address pointer = onPage((Address) cpu->nextByte() + cpu->registers->indexX);
        LSR(cpu, pointer);
        EOR(cpu, cpu->memory->getByte(pointer));
        cpu->cycles += 4;
        return 1;
    }

    int SREInstruction_a(CPU *cpu) {
        Address pointer = cpu->nextAddress();
        LSR(cpu, pointer);
        EOR(cpu, cpu->memory->getByte(pointer));
        cpu->cycles += 4;
        return 2;
    }

    int SREInstruction_a_x(CPU *cpu) {
        Address pointer = cpu->nextAddress() + cpu->registers->indexX;
        LSR(cpu, pointer);
        EOR(cpu, cpu->memory->getByte(pointer));
        cpu->cycles += 5;
        return 2;
    }

    int SREInstruction_a_y(CPU *cpu) {
        Address pointer = cpu->nextAddress() + cpu->registers->indexY;
        LSR(cpu, pointer);
        EOR(cpu, cpu->memory->getByte(pointer));
        cpu->cycles += 5;
        return 2;
    }

    int SREInstruction_$x(CPU *cpu) {
        Address pointer = getAddressOnPage(cpu, (Address) cpu->nextByte() + cpu->registers->indexX);
        LSR(cpu, pointer);
        EOR(cpu, cpu->memory->getByte(pointer));
        cpu->cycles += 6;
        return 1;
    }

    int SREInstruction_$y(CPU *cpu) {
        Address pointer = getAddressOnPage(cpu, (Address) cpu->nextByte()) + cpu->registers->indexY;
        LSR(cpu, pointer);
        EOR(cpu, cpu->memory->getByte(pointer));
        cpu->cycles += 6;
        return 1;
    }

    int ALRInstruction_i(CPU* cpu) {
        AND(cpu, cpu->nextByte());
        LSR(cpu);
        return 1;
    }
    int ANCInstruction_i(CPU* cpu) {
        AND(cpu, cpu->nextByte());
        cpu->setFlags(cpu->isFlagSet(CPURegisters::StatusFlags::Negative), CPURegisters::StatusFlags::Carry);
        return 1;
    }
    int ARRInstruction_i(CPU* cpu) {
        AND(cpu, cpu->nextByte());
        ROR(cpu);
        bool bit6 = (cpu->registers->accumulator & 0b01000000) == 0b01000000;
        bool bit5 = (cpu->registers->accumulator & 0b00100000) == 0b00100000;
        cpu->setFlags(bit6, CPURegisters::StatusFlags::Carry);
        cpu->setFlags(bit6 ^ bit5, CPURegisters::StatusFlags::Overflow);
        return 1;
    }
    int AXSInstruction_i(CPU* cpu) {
        cpu->registers->indexX = (cpu->registers->indexX & cpu->registers->accumulator);
        checkLow(cpu, cpu->registers->indexX);
        cpu->registers->indexX -= cpu->nextByte();
        if (cpu->registers->indexX == cpu->nextByte()) cpu->clearFlags(CPURegisters::StatusFlags::Negative);
        else checkNegative(cpu, cpu->registers->indexX - cpu->nextByte());
        return 1;
    }

    int CLCInstruction(CPU *cpu) {
        cpu->clearFlags(CPURegisters::StatusFlags::Carry);
        return 0;
    }

    int SECInstruction(CPU *cpu) {
        cpu->setFlags(CPURegisters::StatusFlags::Carry);
        return 0;
    }

    int CLIInstruction(CPU *cpu) {
        cpu->clearFlags(CPURegisters::StatusFlags::Interrupt);
        return 0;
    }

    int SEIInstruction(CPU *cpu) {
        cpu->setFlags(CPURegisters::StatusFlags::Interrupt);
        return 0;
    }

    int CLVInstruction(CPU *cpu) {
        cpu->clearFlags(CPURegisters::StatusFlags::Overflow);
        return 0;
    }

    int CLDInstruction(CPU *cpu) {
        cpu->clearFlags(CPURegisters::StatusFlags::Decimal);
        return 0;
    }

    int SEDInstruction(CPU *cpu) {
        cpu->setFlags(CPURegisters::StatusFlags::Decimal);
        return 0;
    }

    int JMPInstruction_a(CPU *cpu) {
        JMP(cpu, cpu->nextAddress());
        cpu->cycles += 1;
        return 0;
    }

    int JMPInstruction_$(CPU *cpu) {
        Address pointer = cpu->nextAddress();
        JMP(cpu, getAddressOnPage(cpu, pointer, hi(pointer)));
        cpu->cycles += 3;
        return 0;
    }

    int BPLInstruction_pd(CPU *cpu) {
        if (!cpu->isFlagSet(CPURegisters::StatusFlags::Negative)) {
            short value = makeSigned(cpu->nextByte());
            if (skippedPage(cpu->registers->programCounter + (Address)2, value)) cpu->cycles += 1;
            cpu->registers->programCounter += value;
            cpu->cycles += 1;
        }
        return 1;
    }

    int BMIInstruction_pd(CPU *cpu) {
        if (cpu->isFlagSet(CPURegisters::StatusFlags::Negative)) {
            short value = makeSigned(cpu->nextByte());
            if (skippedPage(cpu->registers->programCounter + (Address)2, value)) cpu->cycles += 1;
            cpu->registers->programCounter += value;
            cpu->cycles += 1;
        }
        return 1;
    }

    int BVCInstruction_pd(CPU *cpu) {
        if (!cpu->isFlagSet(CPURegisters::StatusFlags::Overflow)) {
            short value = makeSigned(cpu->nextByte());
            if (skippedPage(cpu->registers->programCounter + (Address)2, value)) cpu->cycles += 1;
            cpu->registers->programCounter += value;
            cpu->cycles += 1;
        }
        return 1;
    }

    int BVSInstruction_pd(CPU *cpu) {
        BXS(cpu, CPURegisters::StatusFlags::Overflow, cpu->nextByte());
        return 1;
    }

    int BCCInstruction_pd(CPU *cpu) {
        BXC(cpu, CPURegisters::StatusFlags::Carry, cpu->nextByte());
        return 1;
    }

    int BCSInstruction_pd(CPU *cpu) {
        BXS(cpu, CPURegisters::StatusFlags::Carry, cpu->nextByte());
        return 1;
    }

    int BNEInstruction_pd(CPU *cpu) {
        BXC(cpu, CPURegisters::StatusFlags::Zero, cpu->nextByte());
        return 1;
    }

    int BEQInstruction_pd(CPU *cpu) {
        BXS(cpu, CPURegisters::StatusFlags::Zero, cpu->nextByte());
        return 1;
    }

    int BRKInstruction(CPU *cpu) {
        std::cout << "Break executed. PC: 0x" << makeHex(cpu->registers->programCounter) << std::endl;
#ifdef NEM_PROFILE
        cpu->profiler->breakpoint();
#endif

        cpu->stopExec();

        return 0;
    }

    int RTIInstruction(CPU *cpu) {
        cpu->registers->status = (Byte) (cpu->popByte() | 0b00100000);
        Address pointer = cpu->popAddress();
        cpu->registers->programCounter = pointer;
#ifdef RTI_MINUS_ONE
        cpu->registers->programCounter--;
#endif
        cpu->cycles += 4;
        return 0;
    }

    int JSRInstruction_a(CPU *cpu) {
        cpu->pushAddress(cpu->registers->programCounter + (Address) 2);
        cpu->registers->programCounter = cpu->nextAddress();
        cpu->registers->programCounter--;
        cpu->cycles += 4;
        return 0;
    }

    int RTSInstruction(CPU *cpu) {
        Address address = cpu->popAddress();
        cpu->registers->programCounter = address;
        cpu->cycles += 4;
        return 0;
    }

    int INXInstruction(CPU *cpu) {
        cpu->registers->indexX++;
        checkLow(cpu, cpu->registers->indexX);
        return 0;
    }

    int DEXInstruction(CPU *cpu) {
        cpu->registers->indexX--;
        checkLow(cpu, cpu->registers->indexX);
        return 0;
    }

    int INYInstruction(CPU *cpu) {
        cpu->registers->indexY++;
        checkLow(cpu, cpu->registers->indexY);
        return 0;
    }

    int DEYInstruction(CPU *cpu) {
        cpu->registers->indexY--;
        checkLow(cpu, cpu->registers->indexY);
        return 0;
    }

    int TXAInstruction(CPU *cpu) {
        cpu->registers->accumulator = cpu->registers->indexX;
        checkLow(cpu);
        return 0;
    }

    int TAXInstruction(CPU *cpu) {
        cpu->registers->indexX = cpu->registers->accumulator;
        checkLow(cpu, cpu->registers->indexX);
        return 0;
    }

    int TYAInstruction(CPU *cpu) {
        cpu->registers->accumulator = cpu->registers->indexY;
        checkLow(cpu);
        return 0;
    }

    int TAYInstruction(CPU *cpu) {
        cpu->registers->indexY = cpu->registers->accumulator;
        checkLow(cpu, cpu->registers->indexY);
        return 0;
    }

    int LDAInstruction_i(CPU *cpu) {
        cpu->registers->accumulator = cpu->nextByte();
        checkLow(cpu);
        return 1;
    }

    int LDAInstruction_d(CPU *cpu) {
        cpu->registers->accumulator = cpu->memory->getByte((Address) cpu->nextByte());
        checkLow(cpu);
        cpu->cycles += 1;
        return 1;
    }

    int LDAInstruction_d_x(CPU *cpu) {
        cpu->registers->accumulator = getByteOnPage(cpu, (Address) cpu->nextByte() + cpu->registers->indexX);
        checkLow(cpu);
        cpu->cycles += 2;
        return 1;
    }

    int LDAInstruction_a(CPU *cpu) {
        cpu->registers->accumulator = cpu->memory->getByte(cpu->nextAddress());
        checkLow(cpu);
        cpu->cycles += 2;
        return 2;
    }

    int LDAInstruction_a_x(CPU *cpu) {
        Address pointer = cpu->nextAddress();
        if (skippedPage(pointer, cpu->registers->indexX)) cpu->cycles += 1;
        cpu->registers->accumulator = cpu->memory->getByte(pointer + cpu->registers->indexX);
        checkLow(cpu);
        cpu->cycles += 2;
        return 2;
    }

    int LDAInstruction_a_y(CPU *cpu) {
        Address pointer = cpu->nextAddress();
        if (skippedPage(pointer, cpu->registers->indexY)) cpu->cycles += 1;
        cpu->registers->accumulator = cpu->memory->getByte(pointer + cpu->registers->indexY);
        checkLow(cpu);
        cpu->cycles += 2;
        return 2;
    }

    int LDAInstruction_$x(CPU *cpu) {
        cpu->registers->accumulator = cpu->memory->getByte(
                getAddressOnPage(cpu,
                                 (Address) cpu->nextByte() + cpu->registers->indexX));
        checkLow(cpu);
        cpu->cycles += 4;
        return 1;
    }

    int LDAInstruction_$y(CPU *cpu) {
        Address pointer = getAddressOnPage(cpu, (Address) cpu->nextByte());
        if (skippedPage(pointer, cpu->registers->indexY)) cpu->cycles += 1;
        cpu->registers->accumulator = cpu->memory->getByte(pointer + cpu->registers->indexY);
        checkLow(cpu);
        cpu->cycles += 3;
        return 1;
    }

    int LDXInstruction_i(CPU *cpu) {
        cpu->registers->indexX = cpu->nextByte();
        checkLow(cpu, cpu->registers->indexX);
        return 1;
    }

    int LDXInstruction_d(CPU *cpu) {
        cpu->registers->indexX = cpu->memory->getByte((Address) cpu->nextByte());
        checkLow(cpu, cpu->registers->indexX);
        cpu->cycles += 1;
        return 1;
    }

    int LDXInstruction_d_y(CPU *cpu) {
        cpu->registers->indexX = getByteOnPage(cpu, (Address) cpu->nextByte() + cpu->registers->indexY);
        checkLow(cpu, cpu->registers->indexX);
        cpu->cycles += 2;
        return 1;
    }

    int LDXInstruction_a(CPU *cpu) {
        cpu->registers->indexX = cpu->memory->getByte(cpu->nextAddress());
        checkLow(cpu, cpu->registers->indexX);
        cpu->cycles += 2;
        return 2;
    }

    int LDXInstruction_a_y(CPU *cpu) {
        Address pointer = cpu->nextAddress();
        if (skippedPage(pointer, cpu->registers->indexY)) cpu->cycles += 1;
        cpu->registers->indexX = cpu->memory->getByte(pointer + cpu->registers->indexY);
        checkLow(cpu, cpu->registers->indexX);
        cpu->cycles += 2;
        return 2;
    }

    int LDYInstruction_i(CPU *cpu) {
        cpu->registers->indexY = cpu->nextByte();
        checkLow(cpu, cpu->registers->indexY);
        return 1;
    }

    int LDYInstruction_d(CPU *cpu) {
        cpu->registers->indexY = cpu->memory->getByte((Address) cpu->nextByte());
        checkLow(cpu, cpu->registers->indexY);
        cpu->cycles += 1;
        return 1;
    }

    int LDYInstruction_d_x(CPU *cpu) {
        cpu->registers->indexY = getByteOnPage(cpu, (Address) cpu->nextByte() + cpu->registers->indexX);
        checkLow(cpu, cpu->registers->indexY);
        cpu->cycles += 2;
        return 1;
    }

    int LDYInstruction_a(CPU *cpu) {
        cpu->registers->indexY = cpu->memory->getByte(cpu->nextAddress());
        checkLow(cpu, cpu->registers->indexY);
        cpu->cycles += 2;
        return 2;
    }

    int LDYInstruction_a_x(CPU *cpu) {
        Address pointer = cpu->nextAddress();
        if (skippedPage(pointer, cpu->registers->indexX)) cpu->cycles += 1;
        cpu->registers->indexY = cpu->memory->getByte(pointer + cpu->registers->indexX);
        checkLow(cpu, cpu->registers->indexY);
        cpu->cycles += 2;
        return 2;
    }

    int LAXInstruction_i(CPU *cpu) {
        Byte value = cpu->nextByte();
        cpu->registers->accumulator = value;
        cpu->registers->indexX = value;
        checkLow(cpu);
        return 1;
    }

    int LAXInstruction_d(CPU *cpu) {
        Byte value = cpu->memory->getByte((Address) cpu->nextByte());
        cpu->registers->accumulator = value;
        cpu->registers->indexX = value;
        checkLow(cpu);
        cpu->cycles += 1;
        return 1;
    }

    int LAXInstruction_d_y(CPU *cpu) {
        Byte value = getByteOnPage(cpu, (Address) cpu->nextByte() + cpu->registers->indexY);
        cpu->registers->accumulator = value;
        cpu->registers->indexX = value;
        checkLow(cpu);
        cpu->cycles += 2;
        return 1;
    }

    int LAXInstruction_a(CPU *cpu) {
        Byte value = cpu->memory->getByte(cpu->nextAddress());
        cpu->registers->accumulator = value;
        cpu->registers->indexX = value;
        checkLow(cpu);
        cpu->cycles += 2;
        return 2;
    }

    int LAXInstruction_a_y(CPU *cpu) {
        Byte value = cpu->memory->getByte(cpu->nextAddress() + cpu->registers->indexY);
        cpu->registers->accumulator = value;
        cpu->registers->indexX = value;
        checkLow(cpu);
        cpu->cycles += 2;
        return 2;
    }

    int LAXInstruction_$x(CPU *cpu) {
        Byte value = cpu->memory->getByte(getAddressOnPage(cpu, (Address) cpu->nextByte() + cpu->registers->indexX));
        cpu->registers->accumulator = value;
        cpu->registers->indexX = value;
        checkLow(cpu);
        cpu->cycles += 4;
        return 1;
    }

    int LAXInstruction_$y(CPU *cpu) {
        Address pointer = getAddressOnPage(cpu, (Address) cpu->nextByte());
        if (skippedPage(pointer, cpu->registers->indexY)) cpu->cycles += 1;
        Byte value = cpu->memory->getByte(pointer + cpu->registers->indexY);
        cpu->registers->accumulator = value;
        cpu->registers->indexX = value;
        checkLow(cpu);
        cpu->cycles += 3;
        return 1;
    }

    int STAInstruction_d(CPU *cpu) {
        cpu->memory->setByte((Address) cpu->nextByte(), cpu->registers->accumulator);
        cpu->cycles += 1;
        return 1;
    }

    int STAInstruction_d_x(CPU *cpu) {
        cpu->memory->setByte(onPage((Address) cpu->nextByte() + cpu->registers->indexX), cpu->registers->accumulator);
        cpu->cycles += 2;
        return 1;
    }

    int STAInstruction_a(CPU *cpu) {
        cpu->memory->setByte(cpu->nextAddress(), cpu->registers->accumulator);
        cpu->cycles += 2;
        return 2;
    }

    int STAInstruction_a_x(CPU *cpu) {
        cpu->memory->setByte(cpu->nextAddress() + cpu->registers->indexX, cpu->registers->accumulator);
        cpu->cycles += 3;
        return 2;
    }

    int STAInstruction_a_y(CPU *cpu) {
        cpu->memory->setByte(cpu->nextAddress() + cpu->registers->indexY, cpu->registers->accumulator);
        cpu->cycles += 3;
        return 2;
    }

    int STAInstruction_$x(CPU *cpu) {
        cpu->memory->setByte(getAddressOnPage(cpu,
                                              (Address) cpu->nextByte() + cpu->registers->indexX),
                             cpu->registers->accumulator);
        cpu->cycles += 4;
        return 1;
    }

    int STAInstruction_$y(CPU *cpu) {
        cpu->memory->setByte(getAddressOnPage(cpu,
                                              (Address) cpu->nextByte()) + cpu->registers->indexY,
                             cpu->registers->accumulator);
        cpu->cycles += 4;
        return 1;
    }

    int STXInstruction_d(CPU *cpu) {
        cpu->memory->setByte((Address) cpu->nextByte(), cpu->registers->indexX);
        cpu->cycles += 1;
        return 1;
    }

    int STXInstruction_d_y(CPU *cpu) {
        cpu->memory->setByte(onPage((Address) cpu->nextByte() + cpu->registers->indexY), cpu->registers->indexX);
        cpu->cycles += 2;
        return 1;
    }

    int STXInstruction_a(CPU *cpu) {
        cpu->memory->setByte(cpu->nextAddress(), cpu->registers->indexX);
        cpu->cycles += 2;
        return 2;
    }

    int STYInstruction_d(CPU *cpu) {
        cpu->memory->setByte((Address) cpu->nextByte(), cpu->registers->indexY);
        cpu->cycles += 1;
        return 1;
    }

    int STYInstruction_d_x(CPU *cpu) {
        cpu->memory->setByte(onPage((Address) cpu->nextByte() + cpu->registers->indexX), cpu->registers->indexY);
        cpu->cycles += 2;
        return 1;
    }

    int STYInstruction_a(CPU *cpu) {
        cpu->memory->setByte(cpu->nextAddress(), cpu->registers->indexY);
        cpu->cycles += 2;
        return 2;
    }

    int SAXInstruction_d(CPU *cpu) {
        cpu->memory->setByte((Address) cpu->nextByte(),
                             cpu->registers->accumulator & cpu->registers->indexX);
        cpu->cycles += 1;
        return 1;
    }

    int SAXInstruction_d_y(CPU *cpu) {
        cpu->memory->setByte(onPage((Address) cpu->nextByte() + cpu->registers->indexY),
                             cpu->registers->accumulator & cpu->registers->indexX);
        cpu->cycles += 2;
        return 1;
    }

    int SAXInstruction_a(CPU *cpu) {
        cpu->memory->setByte((Address) cpu->nextAddress(),
                             cpu->registers->accumulator & cpu->registers->indexX);
        cpu->cycles += 2;
        return 2;
    }

    int SAXInstruction_$x(CPU *cpu) {
        cpu->memory->setByte(cpu->memory->getAddress(onPage((Address) cpu->nextByte() + cpu->registers->indexX)),
                             cpu->registers->accumulator & cpu->registers->indexX);
        cpu->cycles += 4;
        return 1;
    }

    int TXSInstruction(CPU *cpu) {
        cpu->registers->stackPointer = cpu->registers->indexX;
        return 0;
    }

    int TSXInstruction(CPU *cpu) {
        cpu->registers->indexX = cpu->registers->stackPointer;
        checkLow(cpu, cpu->registers->indexX);
        return 0;
    }

    int PHAInstruction(CPU *cpu) {
        cpu->pushByte(cpu->registers->accumulator);
        cpu->cycles += 1;
        return 0;
    }

    int PLAInstruction(CPU *cpu) {
        cpu->registers->accumulator = cpu->popByte();
        checkLow(cpu);
        cpu->cycles += 2;
        return 0;
    }

    int PHPInstruction(CPU *cpu) {
        cpu->pushByte((Byte) (cpu->registers->status | 0b00110000));
        cpu->cycles += 1;
        return 0;
    }

    int PLPInstruction(CPU *cpu) {
        cpu->registers->status = (Byte) ((cpu->popByte() & ~0b00010000) | 0b00100000);
        cpu->cycles += 2;
        return 0;
    }

    Instruction opFunctions[] = {
            BRKInstruction, // OP 0x00
            ORAInstruction_$x, // OP 0x01
            Unimplemented, //STPInstruction, // OP 0x02
            SLOInstruction_$x, // OP 0x03
            NOPInstruction_d, // OP 0x04
            ORAInstruction_d, // OP 0x05
            ASLInstruction_d, // OP 0x06
            SLOInstruction_d, // OP 0x07
            PHPInstruction, // OP 0x08
            ORAInstruction_i, // OP 0x09
            ASLInstruction, // OP 0x0A
            ANCInstruction_i, // OP 0x0B
            NOPInstruction_a, // OP 0x0C
            ORAInstruction_a, // OP 0x0D
            ASLInstruction_a, // OP 0x0E
            SLOInstruction_a, // OP 0x0F
            BPLInstruction_pd, // OP 0x10
            ORAInstruction_$y, // OP 0x11
            Unimplemented, //STPInstruction, // OP 0x12
            SLOInstruction_$y, // OP 0x13
            NOPInstruction_d_x, // OP 0x14
            ORAInstruction_d_x, // OP 0x15
            ASLInstruction_d_x, // OP 0x16
            SLOInstruction_d_x, // OP 0x17
            CLCInstruction, // OP 0x18
            ORAInstruction_a_y, // OP 0x19
            NOPInstruction, // OP 0x1A
            SLOInstruction_a_y, // OP 0x1B
            NOPInstruction_a_x, // OP 0x1C
            ORAInstruction_a_x, // OP 0x1D
            ASLInstruction_a_x, // OP 0x1E
            SLOInstruction_a_x, // OP 0x1F
            JSRInstruction_a, // OP 0x20
            ANDInstruction_$x, // OP 0x21
            Unimplemented, //STPInstruction, // OP 0x22
            RLAInstruction_$x, // OP 0x23
            BITInstruction_d, // OP 0x24
            ANDInstruction_d, // OP 0x25
            ROLInstruction_d, // OP 0x26
            RLAInstruction_d, // OP 0x27
            PLPInstruction, // OP 0x28
            ANDInstruction_i, // OP 0x29
            ROLInstruction, // OP 0x2A
            ANCInstruction_i, // OP 0x2B
            BITInstruction_a, // OP 0x2C
            ANDInstruction_a, // OP 0x2D
            ROLInstruction_a, // OP 0x2E
            RLAInstruction_a, // OP 0x2F
            BMIInstruction_pd, // OP 0x30
            ANDInstruction_$y, // OP 0x31
            Unimplemented, //STPInstruction, // OP 0x32
            RLAInstruction_$y, // OP 0x33
            NOPInstruction_d_x, // OP 0x34
            ANDInstruction_d_x, // OP 0x35
            ROLInstruction_d_x, // OP 0x36
            RLAInstruction_d_x, // OP 0x37
            SECInstruction, // OP 0x38
            ANDInstruction_a_y, // OP 0x39
            NOPInstruction, // OP 0x3A
            RLAInstruction_a_y, // OP 0x3B
            NOPInstruction_a_x, // OP 0x3C
            ANDInstruction_a_x, // OP 0x3D
            ROLInstruction_a_x, // OP 0x3E
            RLAInstruction_a_x, // OP 0x3F
            RTIInstruction, // OP 0x40
            EORInstruction_$x, // OP 0x41
            Unimplemented, //STPInstruction, // OP 0x42
            SREInstruction_$x, // OP 0x43
            NOPInstruction_d, // OP 0x44
            EORInstruction_d, // OP 0x45
            LSRInstruction_d, // OP 0x46
            SREInstruction_d, // OP 0x47
            PHAInstruction, // OP 0x48
            EORInstruction_i, // OP 0x49
            LSRInstruction, // OP 0x4A
            ALRInstruction_i, // OP 0x4B
            JMPInstruction_a, // OP 0x4C
            EORInstruction_a, // OP 0x4D
            LSRInstruction_a, // OP 0x4E
            SREInstruction_a, // OP 0x4F
            BVCInstruction_pd, // OP 0x50
            EORInstruction_$y, // OP 0x51
            Unimplemented, //STPInstruction, // OP 0x52
            SREInstruction_$y, // OP 0x53
            NOPInstruction_d_x, // OP 0x54
            EORInstruction_d_x, // OP 0x55
            LSRInstruction_d_x, // OP 0x56
            SREInstruction_d_x, // OP 0x57
            CLIInstruction, // OP 0x58
            EORInstruction_a_y, // OP 0x59
            NOPInstruction, // OP 0x5A
            SREInstruction_a_y, // OP 0x5B
            NOPInstruction_a_x, // OP 0x5C
            EORInstruction_a_x, // OP 0x5D
            LSRInstruction_a_x, // OP 0x5E
            SREInstruction_a_x, // OP 0x5F
            RTSInstruction, // OP 0x60
            ADCInstruction_$x, // OP 0x61
            Unimplemented, //STPInstruction, // OP 0x62
            RRAInstruction_$x, // OP 0x63
            NOPInstruction_d, // OP 0x64
            ADCInstruction_d, // OP 0x65
            RORInstruction_d, // OP 0x66
            RRAInstruction_d, //RRAInstruction_d, // OP 0x67
            PLAInstruction, // OP 0x68
            ADCInstruction_i, // OP 0x69
            RORInstruction, // OP 0x6A
            ARRInstruction_i, // OP 0x6B
            JMPInstruction_$, // OP 0x6C
            ADCInstruction_a, // OP 0x6D
            RORInstruction_a, // OP 0x6E
            RRAInstruction_a, // OP 0x6F
            BVSInstruction_pd, // OP 0x70
            ADCInstruction_$y, // OP 0x71
            Unimplemented, //STPInstruction, // OP 0x72
            RRAInstruction_$y, // OP 0x73
            NOPInstruction_d_x, // OP 0x74
            ADCInstruction_d_x, // OP 0x75
            RORInstruction_d_x, // OP 0x76
            RRAInstruction_d_x, // OP 0x77
            SEIInstruction, // OP 0x78
            ADCInstruction_a_y, // OP 0x79
            NOPInstruction, // OP 0x7A
            RRAInstruction_a_y, // OP 0x7B
            NOPInstruction_a_x, // OP 0x7C
            ADCInstruction_a_x, // OP 0x7D
            RORInstruction_a_x, // OP 0x7E
            RRAInstruction_a_x, // OP 0x7F
            NOPInstruction_i, // OP 0x80
            STAInstruction_$x, // OP 0x81
            NOPInstruction_i, // OP 0x82
            SAXInstruction_$x, // OP 0x83
            STYInstruction_d, // OP 0x84
            STAInstruction_d, // OP 0x85
            STXInstruction_d, // OP 0x86
            SAXInstruction_d, // OP 0x87
            DEYInstruction, // OP 0x88
            NOPInstruction_i, // OP 0x89
            TXAInstruction, // OP 0x8A
            Unimplemented, //XAAInstruction_i, // OP 0x8B
            STYInstruction_a, // OP 0x8C
            STAInstruction_a, // OP 0x8D
            STXInstruction_a, // OP 0x8E
            SAXInstruction_a, //SAXInstruction_a, // OP 0x8F
            BCCInstruction_pd, // OP 0x90
            STAInstruction_$y, // OP 0x91
            Unimplemented, //STPInstruction, // OP 0x92
            Unimplemented, //AHXInstruction_d_y, // OP 0x93
            STYInstruction_d_x, // OP 0x94
            STAInstruction_d_x, // OP 0x95
            STXInstruction_d_y, // OP 0x96
            SAXInstruction_d_y, // OP 0x97
            TYAInstruction, // OP 0x98
            STAInstruction_a_y, // OP 0x99
            TXSInstruction, // OP 0x9A
            Unimplemented, //TASInstruction_a_y, // OP 0x9B
            Unimplemented, //SHYInstruction_a_x, // OP 0x9C
            STAInstruction_a_x, // OP 0x9D
            Unimplemented, //SHXInstruction_a_y, // OP 0x9E
            Unimplemented, //AHXInstruction_a_y, // OP 0x9F
            LDYInstruction_i, // OP 0xA0
            LDAInstruction_$x, // OP 0xA1
            LDXInstruction_i, // OP 0xA2
            LAXInstruction_$x, // OP 0xA3
            LDYInstruction_d, // OP 0xA4
            LDAInstruction_d, // OP 0xA5
            LDXInstruction_d, // OP 0xA6
            LAXInstruction_d, // OP 0xA7
            TAYInstruction, // OP 0xA8
            LDAInstruction_i, // OP 0xA9
            TAXInstruction, // OP 0xAA
            LAXInstruction_i, // OP 0xAB
            LDYInstruction_a, // OP 0xAC
            LDAInstruction_a, // OP 0xAD
            LDXInstruction_a, // OP 0xAE
            LAXInstruction_a, // OP 0xAF
            BCSInstruction_pd, // OP 0xB0
            LDAInstruction_$y, // OP 0xB1
            Unimplemented, //STPInstruction, // OP 0xB2
            LAXInstruction_$y, // OP 0xB3
            LDYInstruction_d_x, // OP 0xB4
            LDAInstruction_d_x, // OP 0xB5
            LDXInstruction_d_y, // OP 0xB6
            LAXInstruction_d_y, // OP 0xB7
            CLVInstruction, // OP 0xB8
            LDAInstruction_a_y, // OP 0xB9
            TSXInstruction, // OP 0xBA
            Unimplemented, //LASInstruction_a_y, // OP 0xBB
            LDYInstruction_a_x, // OP 0xBC
            LDAInstruction_a_x, // OP 0xBD
            LDXInstruction_a_y, // OP 0xBE
            LAXInstruction_a_y, //LAXInstruction_a_y, // OP 0xBF
            CPYInstruction_i, // OP 0xC0
            CMPInstruction_$x, // OP 0xC1
            NOPInstruction_i, // OP 0xC2
            DCPInstruction_$x, // OP 0xC3
            CPYInstruction_d, // OP 0xC4
            CMPInstruction_d, // OP 0xC5
            DECInstruction_d, // OP 0xC6
            DCPInstruction_d, // OP 0xC7
            INYInstruction, // OP 0xC8
            CMPInstruction_i, // OP 0xC9
            DEXInstruction, // OP 0xCA
            AXSInstruction_i, // OP 0xCB
            CPYInstruction_a, // OP 0xCC
            CMPInstruction_a, // OP 0xCD
            DECInstruction_a, // OP 0xCE
            DCPInstruction_a, // OP 0xCF
            BNEInstruction_pd, // OP 0xD0
            CMPInstruction_$y, // OP 0xD1
            Unimplemented, //STPInstruction, // OP 0xD2
            DCPInstruction_$y, // OP 0xD3
            NOPInstruction_d_x, // OP 0xD4
            CMPInstruction_d_x, // OP 0xD5
            DECInstruction_d_x, // OP 0xD6
            DCPInstruction_d_x, // OP 0xD7
            CLDInstruction, // OP 0xD8
            CMPInstruction_a_y, // OP 0xD9
            NOPInstruction, // OP 0xDA
            DCPInstruction_a_y, // OP 0xDB
            NOPInstruction_a_x, // OP 0xDC
            CMPInstruction_a_x, // OP 0xDD
            DECInstruction_a_x, // OP 0xDE
            DCPInstruction_a_x, // OP 0xDF
            CPXInstruction_i, // OP 0xE0
            SBCInstruction_$x, // OP 0xE1
            NOPInstruction_i, // OP 0xE2
            ISBInstruction_$x, // OP 0xE3
            CPXInstruction_d, // OP 0xE4
            SBCInstruction_d, // OP 0xE5
            INCInstruction_d, // OP 0xE6
            ISBInstruction_d, // OP 0xE7
            INXInstruction, // OP 0xE8
            SBCInstruction_i, // OP 0xE9
            NOPInstruction, // OP 0xEA
            SBCInstruction_i, // OP 0xEB
            CPXInstruction_a, // OP 0xEC
            SBCInstruction_a, // OP 0xED
            INCInstruction_a, // OP 0xEE
            ISBInstruction_a, // OP 0xEF
            BEQInstruction_pd, // OP 0xF0
            SBCInstruction_$y, // OP 0xF1
            Unimplemented, //STPInstruction, // OP 0xF2
            ISBInstruction_$y, // OP 0xF3
            NOPInstruction_d_x, // OP 0xF4
            SBCInstruction_d_x, // OP 0xF5
            INCInstruction_d_x, // OP 0xF6
            ISBInstruction_d_x, // OP 0xF7
            SEDInstruction, // OP 0xF8
            SBCInstruction_a_y, // OP 0xF9
            NOPInstruction, // OP 0xFA
            ISBInstruction_a_y, // OP 0xFB
            NOPInstruction_a_x, // OP 0xFC
            SBCInstruction_a_x, // OP 0xFD
            INCInstruction_a_x, // OP 0xFE
            ISBInstruction_a_x, // OP 0xFF
    };
}