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

#define NO_READ(mode) (AddressMode)(mode | NoRead)

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
        checkLow(cpu, cpu->registers.accumulator);
    }

    Address onPage(Address offset, Byte page = 0) {
        return (Address)(offset % 0x100 + page * 0x100);
    }

    Byte getByteOnPage(CPU *cpu, Address offset, Byte page = 0) {
        return cpu->memory.getByte(onPage(offset, page));
    }

    Address getAddressOnPage(CPU *cpu, Address offset, Byte page = 0) {
        Byte a = getByteOnPage(cpu, offset, page);
        Byte b = getByteOnPage(cpu, offset + (Address)1, page);
        return makeAddress(a, b);
    }

    void compare(CPU *cpu, Byte reg, Byte value) {
        cpu->setFlags(reg >= value, CPURegisters::StatusFlags::Carry);
        bool equal = reg == value;
        cpu->setFlags(equal, CPURegisters::StatusFlags::Zero);
        if (equal) cpu->clearFlags(CPURegisters::StatusFlags::Negative);
        else checkNegative(cpu, reg - value);
    }

    void bitCompare(CPU *cpu, Byte value) {
        cpu->setFlags((cpu->registers.accumulator & value) == 0, CPURegisters::StatusFlags::Zero);
        cpu->setFlags((value & 0b01000000) > 0, CPURegisters::StatusFlags::Overflow);
        checkNegative(cpu, value);
    }

    bool unimplemented(CPU* cpu, AddressMode, InstArguments) {
        std::cout << "Instruction not implemented. PC: $" << makeHex(cpu->registers.programCounter)
                  << " INST: $" << makeHex(cpu->memory.getByte(cpu->registers.programCounter)) << std::endl;

#ifdef NEM_PROFILE
        cpu->profiler->breakpoint();
#endif

        cpu->stopExec();

        return true;
    }

    bool nopInstruction(CPU*, AddressMode, InstArguments) { return true; }

    bool adcInstruction(CPU* cpu, AddressMode, InstArguments arguments) {
        bool carry = cpu->isFlagSet(CPURegisters::StatusFlags::Carry);
        bool test = (int) cpu->registers.accumulator + (int) arguments.value + (int) carry > 255;
        cpu->setFlags(
            (bool) (
                ~(cpu->registers.accumulator ^ arguments.value) &
                (cpu->registers.accumulator ^ (cpu->registers.accumulator + arguments.value + (Byte) carry)) & 0x80),
            CPURegisters::StatusFlags::Overflow);
        cpu->registers.accumulator += arguments.value + (Byte) cpu->isFlagSet(CPURegisters::StatusFlags::Carry);
        cpu->setFlags(test, CPURegisters::StatusFlags::Carry);
        checkLow(cpu);
        return true;
    }

    bool sbcInstruction(CPU* cpu, AddressMode mode, InstArguments arguments) {
        arguments.value = ~arguments.value;
        adcInstruction(cpu, mode, arguments);
        return true;
    }

    bool incInstruction(CPU* cpu, AddressMode mode, InstArguments arguments) {
        if (mode == AbsoluteX && !arguments.skipped) cpu->readCycle();
        //if (!arguments.faster) cpu->readCycle();
        arguments.value++;
        cpu->writeCycle();
        cpu->memory.setByte(arguments.pointer, arguments.value);
        checkLow(cpu, arguments.value);
        return true;
    }

    bool decInstruction(CPU* cpu, AddressMode mode, InstArguments arguments) {
        if (mode == AbsoluteX && !arguments.skipped) cpu->readCycle();
        arguments.value--;
        cpu->writeCycle();
        cpu->memory.setByte(arguments.pointer, arguments.value);
        checkLow(cpu, arguments.value);
        return true;
    }

    bool isbInstruction(CPU *cpu, AddressMode mode, InstArguments arguments) {
        incInstruction(cpu, mode, arguments);
        arguments.value = cpu->memory.getByte(arguments.pointer, false);
        sbcInstruction(cpu, mode, arguments);
        return true;
    }

    bool dcpInstruction(CPU* cpu, AddressMode mode, InstArguments arguments) {
        decInstruction(cpu, mode, arguments);
        arguments.value = cpu->memory.getByte(arguments.pointer, false);
        compare(cpu, cpu->registers.accumulator, arguments.value);
        return true;
    }

    bool andInstruction(CPU* cpu, AddressMode, InstArguments arguments) {
        cpu->registers.accumulator &= arguments.value;
        checkLow(cpu);
        return true;
    }

    bool oraInstruction(CPU* cpu, AddressMode, InstArguments arguments) {
        cpu->registers.accumulator |= arguments.value;
        checkLow(cpu);
        return true;
    }

    bool eorInstruction(CPU* cpu, AddressMode, InstArguments arguments) {
        cpu->registers.accumulator ^= arguments.value;
        checkLow(cpu);
        return true;
    }

    bool bitInstruction(CPU* cpu, AddressMode, InstArguments arguments) {
        bitCompare(cpu, arguments.value);
        return true;
    }

    bool cmpInstruction(CPU* cpu, AddressMode mode, InstArguments arguments) {
        compare(cpu, cpu->registers.accumulator, arguments.value);
        return true;
    }

    bool cpxInstruction(CPU* cpu, AddressMode mode, InstArguments arguments) {
        compare(cpu, cpu->registers.indexX, arguments.value);
        return true;
    }

    bool cpyInstruction(CPU* cpu, AddressMode mode, InstArguments arguments) {
        compare(cpu, cpu->registers.indexY, arguments.value);
        return true;
    }

    bool aslInstruction(CPU* cpu, AddressMode mode, InstArguments arguments) {
        if (mode == Implied) {
            cpu->setFlags(((int) cpu->registers.accumulator << 1) > 255, CPURegisters::StatusFlags::Carry);
            cpu->registers.accumulator = cpu->registers.accumulator << 1;
            checkLow(cpu);
        } else {
            if (mode == AbsoluteX && !arguments.skipped) cpu->readCycle();
            //if (!arguments.faster) cpu->readCycle();
            cpu->setFlags((arguments.value & 0b10000000) == 0b10000000, CPURegisters::StatusFlags::Carry);
            cpu->memory.setByte(arguments.pointer, arguments.value << 1);
            cpu->writeCycle();
            checkLow(cpu, arguments.value << 1);
        }
        return true;
    }

    bool lsrInstruction(CPU* cpu, AddressMode mode, InstArguments arguments) {
        if (mode == Implied) {
            cpu->setFlags((cpu->registers.accumulator & 0b00000001) == 0b00000001, CPURegisters::StatusFlags::Carry);
            cpu->registers.accumulator = cpu->registers.accumulator >> 1;
            checkLow(cpu);
        } else {
            if (mode == AbsoluteX && !arguments.skipped) cpu->readCycle();
            //if (!arguments.faster) cpu->readCycle();
            cpu->setFlags((arguments.value & 0b00000001) == 0b00000001, CPURegisters::StatusFlags::Carry);
            cpu->memory.setByte(arguments.pointer, arguments.value >> 1);
            cpu->writeCycle();
            checkLow(cpu, arguments.value >> 1);
        }
        return true;
    }

    bool rolInstruction(CPU* cpu, AddressMode mode, InstArguments arguments) {
        if (mode == Implied) {
            Byte bit0 = cpu->isFlagSet(CPURegisters::StatusFlags::Carry) ? (Byte) 0b00000001 : (Byte) 0b00000000;
            cpu->setFlags((cpu->registers.accumulator & 0b10000000) == 0b10000000, CPURegisters::StatusFlags::Carry);
            cpu->registers.accumulator = (cpu->registers.accumulator << 1) | bit0;
            checkLow(cpu);
        } else {
            if (mode == AbsoluteX && !arguments.skipped) cpu->readCycle();
            //if (!arguments.faster) cpu->readCycle();
            Byte bit0 = cpu->isFlagSet(CPURegisters::StatusFlags::Carry) ? (Byte) 0b00000001 : (Byte) 0b00000000;
            cpu->setFlags((arguments.value & 0b10000000) == 0b10000000, CPURegisters::StatusFlags::Carry);
            arguments.value = (arguments.value << 1) | bit0;
            cpu->memory.setByte(arguments.pointer, arguments.value);
            cpu->writeCycle();
            checkLow(cpu, arguments.value);
        }
        return true;
    }

    bool rorInstruction(CPU* cpu, AddressMode mode, InstArguments arguments) {
        if (mode == Implied) {
            Byte bit7 = cpu->isFlagSet(CPURegisters::StatusFlags::Carry) ? (Byte) 0b10000000 : (Byte) 0b00000000;
            cpu->setFlags((cpu->registers.accumulator & 0b00000001) == 0b00000001, CPURegisters::StatusFlags::Carry);
            cpu->registers.accumulator = (cpu->registers.accumulator >> 1) | bit7;
            checkLow(cpu);
        } else {
            if (mode == AbsoluteX && !arguments.skipped) cpu->readCycle();
            //if (!arguments.faster) cpu->readCycle();
            Byte bit7 = cpu->isFlagSet(CPURegisters::StatusFlags::Carry) ? (Byte) 0b10000000 : (Byte) 0b00000000;
            cpu->setFlags((arguments.value & 0b00000001) == 0b00000001, CPURegisters::StatusFlags::Carry);
            arguments.value = (arguments.value >> 1) | bit7;
            cpu->memory.setByte(arguments.pointer, arguments.value);
            cpu->writeCycle();
            checkLow(cpu, arguments.value);
        }
        return true;
    }

    bool sloInstruction(CPU* cpu, AddressMode mode, InstArguments arguments) {
        aslInstruction(cpu, mode, arguments);
        arguments.value = cpu->memory.getByte(arguments.pointer, false);
        oraInstruction(cpu, mode, arguments);
        return true;
    }

    bool sreInstruction(CPU* cpu, AddressMode mode, InstArguments arguments) {
        lsrInstruction(cpu, mode, arguments);
        arguments.value = cpu->memory.getByte(arguments.pointer, false);
        eorInstruction(cpu, mode, arguments);
        return true;
    }

    bool rlaInstruction(CPU* cpu, AddressMode mode, InstArguments arguments) {
        rolInstruction(cpu, mode, arguments);
        arguments.value = cpu->memory.getByte(arguments.pointer, false);
        andInstruction(cpu, mode, arguments);
        return true;
    }

    bool rraInstruction(CPU* cpu, AddressMode mode, InstArguments arguments) {
        rorInstruction(cpu, mode, arguments);
        arguments.value = cpu->memory.getByte(arguments.pointer, false);
        adcInstruction(cpu, mode, arguments);
        return true;
    }

    bool alrInstruction(CPU* cpu, AddressMode mode, InstArguments arguments) {
        andInstruction(cpu, mode, arguments);
        lsrInstruction(cpu, Implied, arguments);
        return true;
    }
    bool ancInstruction(CPU* cpu, AddressMode mode, InstArguments arguments) {
        andInstruction(cpu, mode, arguments);
        cpu->setFlags(cpu->isFlagSet(CPURegisters::StatusFlags::Negative), CPURegisters::StatusFlags::Carry);
        return true;
    }
    bool arrInstruction(CPU* cpu, AddressMode mode, InstArguments arguments) {
        andInstruction(cpu, mode, arguments);
        rorInstruction(cpu, Implied, arguments);
        bool bit6 = (cpu->registers.accumulator & 0b01000000) == 0b01000000;
        bool bit5 = (cpu->registers.accumulator & 0b00100000) == 0b00100000;
        cpu->setFlags(bit6, CPURegisters::StatusFlags::Carry);
        cpu->setFlags(bit6 ^ bit5, CPURegisters::StatusFlags::Overflow);
        return true;
    }
    bool axsInstruction(CPU* cpu, AddressMode, InstArguments arguments) {
        cpu->registers.indexX = (cpu->registers.indexX & cpu->registers.accumulator);
        checkLow(cpu, cpu->registers.indexX);
        cpu->registers.indexX -= arguments.value;
        if (cpu->registers.indexX == arguments.value) cpu->clearFlags(CPURegisters::StatusFlags::Negative);
        else checkNegative(cpu, cpu->registers.indexX - arguments.value);
        return true;
    }

    bool scxInstruction(CPU* cpu, AddressMode, InstArguments) {
        bool value = true;
        CPURegisters::StatusFlags flags = (CPURegisters::StatusFlags)0;
        switch (cpu->thisByte(false)) {
            case 0x18: value = false;
            case 0x38: flags = CPURegisters::StatusFlags::Carry; break;
            case 0x58: value = false;
            case 0x78: flags = CPURegisters::StatusFlags::Interrupt; break;
            case 0xB8: value = false; flags = CPURegisters::StatusFlags::Overflow; break;
            case 0xD8: value = false;
            case 0xF8: flags = CPURegisters::StatusFlags::Decimal; break;
            default: break;
        }
        cpu->setFlags(value, flags);
        return true;
    }

    bool jmpInstruction(CPU* cpu, AddressMode, InstArguments arguments) {
        cpu->registers.programCounter = arguments.pointer - (Address)1;
        return false;
    }

    bool bxxInstruction(CPU* cpu, AddressMode, InstArguments arguments) {
        bool req = true;
        CPURegisters::StatusFlags flags = (CPURegisters::StatusFlags)0;
        switch (cpu->thisByte(false)) {
            case 0x10: req = false;
            case 0x30: flags = CPURegisters::StatusFlags::Negative; break;
            case 0x50: req = false;
            case 0x70: flags = CPURegisters::StatusFlags::Overflow; break;
            case 0x90: req = false;
            case 0xB0: flags = CPURegisters::StatusFlags::Carry; break;
            case 0xD0: req = false;
            case 0xF0: flags = CPURegisters::StatusFlags::Zero; break;
            default: break;
        }
        if (cpu->isFlagSet(flags) == req) {
            cpu->readCycle();
            short value = makeSigned(arguments.value);
            if (skippedPage(cpu->registers.programCounter + (Address)2, value)) cpu->readCycle();
            cpu->registers.programCounter += value;
        }
        return true;
    }

    bool brkInstruction(CPU* cpu, AddressMode, InstArguments) {
        std::cout << "Break executed. PC: 0x" << makeHex(cpu->registers.programCounter) << std::endl;

#ifdef NEM_PROFILE
        cpu->profiler->breakpoint();
#endif

        cpu->stopExec();
        return true;
    }

    bool stpInstruction(CPU* cpu, AddressMode, InstArguments) {
        std::cout << "Halt executed. PC: 0x" << makeHex(cpu->registers.programCounter) << std::endl;

#ifdef NEM_PROFILE
        cpu->profiler->breakpoint();
#endif

        cpu->stopExec();
        return true;
    }

    bool rtiInstruction(CPU* cpu, AddressMode, InstArguments) {
        cpu->registers.status = (Byte) (cpu->popByte() | 0b00100000);
        Address pointer = cpu->popAddress();
        cpu->writeCycle();
        cpu->registers.programCounter = pointer;
#ifdef RTI_MINUS_ONE
        cpu->registers.programCounter--;
#endif
        return false;
    }

    bool jsrInstruction(CPU* cpu, AddressMode, InstArguments arguments) {
        cpu->pushAddress(cpu->registers.programCounter + (Address) 2);
        cpu->writeCycle();
        cpu->registers.programCounter = arguments.pointer;
        cpu->registers.programCounter--;
        return false;
    }

    bool rtsInstruction(CPU* cpu, AddressMode, InstArguments) {
        Address address = cpu->popAddress();
        cpu->readCycle(); cpu->writeCycle();
        cpu->registers.programCounter = address;
        return false;
    }

    bool inxInstruction(CPU *cpu, AddressMode, InstArguments) {
        cpu->registers.indexX++;
        checkLow(cpu, cpu->registers.indexX);
        return false;
    }

    bool dexInstruction(CPU *cpu, AddressMode, InstArguments) {
        cpu->registers.indexX--;
        checkLow(cpu, cpu->registers.indexX);
        return false;
    }

    bool inyInstruction(CPU *cpu, AddressMode, InstArguments) {
        cpu->registers.indexY++;
        checkLow(cpu, cpu->registers.indexY);
        return false;
    }

    bool deyInstruction(CPU *cpu, AddressMode, InstArguments) {
        cpu->registers.indexY--;
        checkLow(cpu, cpu->registers.indexY);
        return false;
    }

    bool txaInstruction(CPU *cpu, AddressMode, InstArguments) {
        cpu->registers.accumulator = cpu->registers.indexX;
        checkLow(cpu);
        return false;
    }

    bool taxInstruction(CPU *cpu, AddressMode, InstArguments) {
        cpu->registers.indexX = cpu->registers.accumulator;
        checkLow(cpu, cpu->registers.indexX);
        return false;
    }

    bool tyaInstruction(CPU *cpu, AddressMode, InstArguments) {
        cpu->registers.accumulator = cpu->registers.indexY;
        checkLow(cpu);
        return false;
    }

    bool tayInstruction(CPU *cpu, AddressMode, InstArguments) {
        cpu->registers.indexY = cpu->registers.accumulator;
        checkLow(cpu, cpu->registers.indexY);
        return false;
    }

    bool ldaInstruction(CPU* cpu, AddressMode, InstArguments arguments) {
        cpu->registers.accumulator = arguments.value;
        checkLow(cpu);
        return true;
    }

    bool ldxInstruction(CPU* cpu, AddressMode, InstArguments arguments) {
        cpu->registers.indexX = arguments.value;
        checkLow(cpu, cpu->registers.indexX);
        return true;
    }

    bool ldyInstruction(CPU* cpu, AddressMode, InstArguments arguments) {
        cpu->registers.indexY = arguments.value;
        checkLow(cpu, cpu->registers.indexY);
        return true;
    }

    bool laxInstruction(CPU* cpu, AddressMode mode, InstArguments arguments) {
        cpu->registers.accumulator = arguments.value;
        cpu->registers.indexX = arguments.value;
        checkLow(cpu);
        return true;
    }

    bool staInstruction(CPU* cpu, AddressMode mode, InstArguments arguments) {
        if ((mode == AbsoluteX ||
            mode == AbsoluteY ||
            mode == IndirectY) && !arguments.skipped) cpu->readCycle();
        cpu->memory.setByte(arguments.pointer, cpu->registers.accumulator);
        return true;
    }

    bool stxInstruction(CPU* cpu, AddressMode, InstArguments arguments) {
        cpu->memory.setByte(arguments.pointer, cpu->registers.indexX);
        return true;
    }

    bool styInstruction(CPU* cpu, AddressMode, InstArguments arguments) {
        cpu->memory.setByte(arguments.pointer, cpu->registers.indexY);
        return true;
    }

    bool saxInstruction(CPU* cpu, AddressMode, InstArguments arguments) {
        cpu->memory.setByte(arguments.pointer, cpu->registers.accumulator & cpu->registers.indexX);
        return true;
    }

    bool txsInstruction(CPU* cpu, AddressMode, InstArguments) {
        cpu->registers.stackPointer = cpu->registers.indexX;
        return true;
    }

    bool tsxInstruction(CPU* cpu, AddressMode, InstArguments) {
        cpu->registers.indexX = cpu->registers.stackPointer;
        checkLow(cpu, cpu->registers.indexX);
        return true;
    }

    bool phaInstruction(CPU* cpu, AddressMode, InstArguments) {
        cpu->pushByte(cpu->registers.accumulator);
        return true;
    }

    bool plaInstruction(CPU* cpu, AddressMode, InstArguments) {
        cpu->registers.accumulator = cpu->popByte();
        cpu->writeCycle();
        checkLow(cpu);
        return true;
    }

    bool phpInstruction(CPU* cpu, AddressMode, InstArguments) {
        cpu->pushByte((Byte) (cpu->registers.status | 0b00110000));
        return true;
    }

    bool plpInstruction(CPU* cpu, AddressMode, InstArguments) {
        cpu->registers.status = (Byte) ((cpu->popByte() & ~0b00010000) | 0b00100000);
        cpu->writeCycle();
        return true;
    }

    int callInstruction(Instruction &inst, CPU *cpu) {
        InstArguments arguments = { 0, 0, false };
        bool read = (inst.mode & NoRead) != NoRead;
        AddressMode mode = (AddressMode)(inst.mode & 0b00001111);
        int length = addressModeLengths[mode];
        Byte next = 0;
        Address nextA = 0;
        if (length <= 2) next = cpu->nextByte();
        else if (length >= 3) nextA = cpu->nextAddress();
        switch (mode) {
            case Implied: break;
            case Immediate:
                arguments.value = next;
                break;
            case ZeroPage:
                if (read) arguments.value = cpu->memory.getByte((Address)next);
                arguments.pointer = (Address)next;
                break;
            case ZeroPageX:
                cpu->readCycle();
                arguments.pointer = onPage((Address)next + cpu->registers.indexX);
                if (read) arguments.value = cpu->memory.getByte(arguments.pointer);
                break;
            case ZeroPageY:
                cpu->readCycle();
                arguments.pointer = onPage((Address)next + cpu->registers.indexY);
                if (read) arguments.value = cpu->memory.getByte(arguments.pointer);
                break;
            case Absolute:
                if (read) arguments.value = cpu->memory.getByte(nextA);
                arguments.pointer = nextA;
                break;
            case AbsoluteX:
                arguments.pointer = nextA;
                if (skippedPage(arguments.pointer, cpu->registers.indexX)) {
                    cpu->readCycle();
                    arguments.skipped = true;
                }
                arguments.pointer += cpu->registers.indexX;
                if (read) arguments.value = cpu->memory.getByte(arguments.pointer);
                break;
            case AbsoluteY:
                arguments.pointer = nextA;
                if (skippedPage(arguments.pointer, cpu->registers.indexY)) {
                    cpu->readCycle();
                    arguments.skipped = true;
                }
                arguments.pointer += cpu->registers.indexY;
                if (read) arguments.value = cpu->memory.getByte(arguments.pointer);
                break;
            case IndirectX:
                cpu->readCycle();
                arguments.pointer = getAddressOnPage(cpu, (Address)next + cpu->registers.indexX);
                if (read) arguments.value = cpu->memory.getByte(arguments.pointer);
                break;
            case IndirectY:
                arguments.pointer = getAddressOnPage(cpu, (Address)next);
                if (skippedPage(arguments.pointer, cpu->registers.indexY)) {
                    cpu->readCycle();
                    arguments.skipped = true;
                }
                arguments.pointer += cpu->registers.indexY;
                if (read) arguments.value = cpu->memory.getByte(arguments.pointer);
                break;
            case IndirectAbsolute:
                arguments.pointer = getAddressOnPage(cpu, nextA, hi(nextA));
                break;
            case Relative:
                arguments.value = next;
                break;
            case Unknown:
#ifdef NEM_PROFILER
                cpu->profiler->breakpoint();
#endif
                std::cout << "Unknown Address Mode." << std::endl;
                break;
            default: break;
        }
        return inst.function(cpu, mode, arguments) ? addressModeLengths[mode] : 1;
    }

    Instruction opInstructions[] = {
        { brkInstruction, "BRK", Implied }, // OP 0x00
        { oraInstruction, "ORA", IndirectX }, // OP 0x01
        { stpInstruction, "STP", Implied }, // OP 0x02
        { sloInstruction, "SLO", IndirectX }, // OP 0x03
        { nopInstruction, "NOP", ZeroPage }, // OP 0x04
        { oraInstruction, "ORA", ZeroPage }, // OP 0x05
        { aslInstruction, "ASL", ZeroPage }, // OP 0x06
        { sloInstruction, "SLO", ZeroPage }, // OP 0x07
        { phpInstruction, "PHP", Implied }, // OP 0x08
        { oraInstruction, "ORA", Immediate }, // OP 0x09
        { aslInstruction, "ASL", Implied }, // OP 0x0A
        { ancInstruction, "ANC", Immediate }, // OP 0x0B
        { nopInstruction, "NOP", Absolute }, // OP 0x0C
        { oraInstruction, "ORA", Absolute }, // OP 0x0D
        { aslInstruction, "ASL", Absolute }, // OP 0x0E
        { sloInstruction, "SLO", Absolute }, // OP 0x0F
        { bxxInstruction, "BPL", Relative }, // OP 0x10
        { oraInstruction, "ORA", IndirectY }, // OP 0x11
        { stpInstruction, "STP", Implied }, // OP 0x12
        { sloInstruction, "SLO", IndirectY }, // OP 0x13
        { nopInstruction, "NOP", ZeroPageX }, // OP 0x14
        { oraInstruction, "ORA", ZeroPageX }, // OP 0x15
        { aslInstruction, "ASL", ZeroPageX }, // OP 0x16
        { sloInstruction, "SLO", ZeroPageX }, // OP 0x17
        { scxInstruction, "CLC", Implied }, // OP 0x18
        { oraInstruction, "ORA", AbsoluteY }, // OP 0x19
        { nopInstruction, "NOP", Implied }, // OP 0x1A
        { sloInstruction, "SLO", AbsoluteY }, // OP 0x1B
        { nopInstruction, "NOP", AbsoluteX }, // OP 0x1C
        { oraInstruction, "ORA", AbsoluteX }, // OP 0x1D
        { aslInstruction, "ASL", AbsoluteX }, // OP 0x1E
        { sloInstruction, "SLO", AbsoluteX }, // OP 0x1F
        { jsrInstruction, "JSR", NO_READ(Absolute) }, // OP 0x20
        { andInstruction, "AND", IndirectX }, // OP 0x21
        { stpInstruction, "STP", Implied }, // OP 0x22
        { rlaInstruction, "RLA", IndirectX }, // OP 0x23
        { bitInstruction, "BIT", ZeroPage }, // OP 0x24
        { andInstruction, "AND", ZeroPage }, // OP 0x25
        { rolInstruction, "ROL", ZeroPage }, // OP 0x26
        { rlaInstruction, "RLA", ZeroPage }, // OP 0x27
        { plpInstruction, "PLP", Implied }, // OP 0x28
        { andInstruction, "AND", Immediate }, // OP 0x29
        { rolInstruction, "ROL", Implied }, // OP 0x2A
        { ancInstruction, "ANC", Immediate }, // OP 0x2B
        { bitInstruction, "BIT", Absolute }, // OP 0x2C
        { andInstruction, "AND", Absolute }, // OP 0x2D
        { rolInstruction, "ROL", Absolute }, // OP 0x2E
        { rlaInstruction, "RLA", Absolute }, // OP 0x2F
        { bxxInstruction, "BMI", Relative }, // OP 0x30
        { andInstruction, "AND", IndirectY }, // OP 0x31
        { stpInstruction, "STP", Implied }, // OP 0x32
        { rlaInstruction, "RLA", IndirectY }, // OP 0x33
        { nopInstruction, "NOP", ZeroPageX }, // OP 0x34
        { andInstruction, "AND", ZeroPageX }, // OP 0x35
        { rolInstruction, "ROL", ZeroPageX }, // OP 0x36
        { rlaInstruction, "RLA", ZeroPageX }, // OP 0x37
        { scxInstruction, "SEC", Implied }, // OP 0x38
        { andInstruction, "AND", AbsoluteY }, // OP 0x39
        { nopInstruction, "NOP", Implied }, // OP 0x3A
        { rlaInstruction, "RLA", AbsoluteY }, // OP 0x3B
        { nopInstruction, "NOP", AbsoluteX }, // OP 0x3C
        { andInstruction, "AND", AbsoluteX }, // OP 0x3D
        { rolInstruction, "ROL", AbsoluteX }, // OP 0x3E
        { rlaInstruction, "RLA", AbsoluteX }, // OP 0x3F
        { rtiInstruction, "RTI", Implied }, // OP 0x40
        { eorInstruction, "EOR", IndirectX }, // OP 0x41
        { stpInstruction, "STP", Implied }, // OP 0x42
        { sreInstruction, "SRE", IndirectX }, // OP 0x43
        { nopInstruction, "NOP", ZeroPage }, // OP 0x44
        { eorInstruction, "EOR", ZeroPage }, // OP 0x45
        { lsrInstruction, "LSR", ZeroPage }, // OP 0x46
        { sreInstruction, "SRE", ZeroPage }, // OP 0x47
        { phaInstruction, "PHA", Implied }, // OP 0x48
        { eorInstruction, "EOR", Immediate }, // OP 0x49
        { lsrInstruction, "LSR", Implied }, // OP 0x4A
        { alrInstruction, "ALR", Immediate }, // OP 0x4B
        { jmpInstruction, "JMP", NO_READ(Absolute) }, // OP 0x4C
        { eorInstruction, "EOR", Absolute }, // OP 0x4D
        { lsrInstruction, "LSR", Absolute }, // OP 0x4E
        { sreInstruction, "SRE", Absolute }, // OP 0x4F
        { bxxInstruction, "BVC", Relative }, // OP 0x50
        { eorInstruction, "EOR", IndirectY }, // OP 0x51
        { stpInstruction, "STP", Implied }, // OP 0x52
        { sreInstruction, "SRE", IndirectY }, // OP 0x53
        { nopInstruction, "NOP", ZeroPageX }, // OP 0x54
        { eorInstruction, "EOR", ZeroPageX }, // OP 0x55
        { lsrInstruction, "LSR", ZeroPageX }, // OP 0x56
        { sreInstruction, "SRE", ZeroPageX }, // OP 0x57
        { scxInstruction, "CLI", Implied }, // OP 0x58
        { eorInstruction, "EOR", AbsoluteY }, // OP 0x59
        { nopInstruction, "NOP", Implied }, // OP 0x5A
        { sreInstruction, "SRE", AbsoluteY }, // OP 0x5B
        { nopInstruction, "NOP", AbsoluteX }, // OP 0x5C
        { eorInstruction, "EOR", AbsoluteX }, // OP 0x5D
        { lsrInstruction, "LSR", AbsoluteX }, // OP 0x5E
        { sreInstruction, "SRE", AbsoluteX }, // OP 0x5F
        { rtsInstruction, "RTS", Implied }, // OP 0x60
        { adcInstruction, "ADC", IndirectX }, // OP 0x61
        { stpInstruction, "STP", Implied }, // OP 0x62
        { rraInstruction, "RRA", IndirectX }, // OP 0x63
        { nopInstruction, "NOP", ZeroPage }, // OP 0x64
        { adcInstruction, "ADC", ZeroPage }, // OP 0x65
        { rorInstruction, "ROR", ZeroPage }, // OP 0x66
        { rraInstruction, "RRA", ZeroPage }, // OP 0x67
        { plaInstruction, "PLA", Implied }, // OP 0x68
        { adcInstruction, "ADC", Immediate }, // OP 0x69
        { rorInstruction, "ROR", Implied }, // OP 0x6A
        { arrInstruction, "ARR", Immediate }, // OP 0x6B
        { jmpInstruction, "JMP", NO_READ(IndirectAbsolute) }, // OP 0x6C
        { adcInstruction, "ADC", Absolute }, // OP 0x6D
        { rorInstruction, "ROR", Absolute }, // OP 0x6E
        { rraInstruction, "RRA", Absolute }, // OP 0x6F
        { bxxInstruction, "BVS", Relative }, // OP 0x70
        { adcInstruction, "ADC", IndirectY }, // OP 0x71
        { stpInstruction, "STP", Implied }, // OP 0x72
        { rraInstruction, "RRA", IndirectY }, // OP 0x73
        { nopInstruction, "NOP", ZeroPageX }, // OP 0x74
        { adcInstruction, "ADC", ZeroPageX }, // OP 0x75
        { rorInstruction, "ROR", ZeroPageX }, // OP 0x76
        { rraInstruction, "RRA", ZeroPageX }, // OP 0x77
        { scxInstruction, "SEI", Implied }, // OP 0x78
        { adcInstruction, "ADC", AbsoluteY }, // OP 0x79
        { nopInstruction, "NOP", Implied }, // OP 0x7A
        { rraInstruction, "RRA", AbsoluteY }, // OP 0x7B
        { nopInstruction, "NOP", AbsoluteX }, // OP 0x7C
        { adcInstruction, "ADC", AbsoluteX }, // OP 0x7D
        { rorInstruction, "ROR", AbsoluteX }, // OP 0x7E
        { rraInstruction, "RRA", AbsoluteX }, // OP 0x7F
        { nopInstruction, "NOP", Immediate }, // OP 0x80
        { staInstruction, "STA", NO_READ(IndirectX) }, // OP 0x81
        { nopInstruction, "NOP", Immediate }, // OP 0x82
        { saxInstruction, "SAX", NO_READ(IndirectX) }, // OP 0x83
        { styInstruction, "STY", NO_READ(ZeroPage) }, // OP 0x84
        { staInstruction, "STA", NO_READ(ZeroPage) }, // OP 0x85
        { stxInstruction, "STX", NO_READ(ZeroPage) }, // OP 0x86
        { saxInstruction, "SAX", NO_READ(ZeroPage) }, // OP 0x87
        { deyInstruction, "DEY", Implied }, // OP 0x88
        { nopInstruction, "NOP", Immediate }, // OP 0x89
        { txaInstruction, "TXA", Implied }, // OP 0x8A
        { unimplemented, "UNI", Unknown }, //XAAInstruction_i, // OP 0x8B
        { styInstruction, "STY", NO_READ(Absolute) }, // OP 0x8C
        { staInstruction, "STA", NO_READ(Absolute) }, // OP 0x8D
        { stxInstruction, "STX", NO_READ(Absolute) }, // OP 0x8E
        { saxInstruction, "SAX", NO_READ(Absolute) }, // OP 0x8F
        { bxxInstruction, "BCC", Relative }, // OP 0x90
        { staInstruction, "STA", NO_READ(IndirectY) }, // OP 0x91
        { stpInstruction, "STP", Implied }, // OP 0x92
        { unimplemented, "UNI", Unknown }, //AHXInstruction_d_y, // OP 0x93
        { styInstruction, "STY", NO_READ(ZeroPageX) }, // OP 0x94
        { staInstruction, "STA", NO_READ(ZeroPageX) }, // OP 0x95
        { stxInstruction, "STX", NO_READ(ZeroPageY) }, // OP 0x96
        { saxInstruction, "SAX", NO_READ(ZeroPageY) }, // OP 0x97
        { tyaInstruction, "TYA", Implied }, // OP 0x98
        { staInstruction, "STA", NO_READ(AbsoluteY) }, // OP 0x99
        { txsInstruction, "TXS", Implied }, // OP 0x9A
        { unimplemented, "UNI", Unknown }, //TASInstruction_a_y, // OP 0x9B
        { unimplemented, "UNI", Unknown }, //SHYInstruction_a_x, // OP 0x9C
        { staInstruction, "STA", NO_READ(AbsoluteX) }, // OP 0x9D
        { unimplemented, "UNI", Unknown }, //SHXInstruction_a_y, // OP 0x9E
        { unimplemented, "UNI", Unknown }, //AHXInstruction_a_y, // OP 0x9F
        { ldyInstruction, "LDY", Immediate }, // OP 0xA0
        { ldaInstruction, "LDA", IndirectX }, // OP 0xA1
        { ldxInstruction, "LDX", Immediate }, // OP 0xA2
        { laxInstruction, "LAX", IndirectX }, // OP 0xA3
        { ldyInstruction, "LDY", ZeroPage }, // OP 0xA4
        { ldaInstruction, "LDA", ZeroPage }, // OP 0xA5
        { ldxInstruction, "LDX", ZeroPage }, // OP 0xA6
        { laxInstruction, "LAX", ZeroPage }, // OP 0xA7
        { tayInstruction, "TAY", Implied }, // OP 0xA8
        { ldaInstruction, "LDA", Immediate }, // OP 0xA9
        { taxInstruction, "TAX", Implied }, // OP 0xAA
        { laxInstruction, "LAX", Immediate }, // OP 0xAB
        { ldyInstruction, "LDY", Absolute }, // OP 0xAC
        { ldaInstruction, "LDA", Absolute }, // OP 0xAD
        { ldxInstruction, "LDX", Absolute }, // OP 0xAE
        { laxInstruction, "LAX", Absolute }, // OP 0xAF
        { bxxInstruction, "BCS", Relative }, // OP 0xB0
        { ldaInstruction, "LDA", IndirectY }, // OP 0xB1
        { stpInstruction, "STP", Implied }, // OP 0xB2
        { laxInstruction, "LAX", IndirectY }, // OP 0xB3
        { ldyInstruction, "LDY", ZeroPageX }, // OP 0xB4
        { ldaInstruction, "LDA", ZeroPageX }, // OP 0xB5
        { ldxInstruction, "LDX", ZeroPageY }, // OP 0xB6
        { laxInstruction, "LAX", ZeroPageY }, // OP 0xB7
        { scxInstruction, "CLV", Implied }, // OP 0xB8
        { ldaInstruction, "LDA", AbsoluteY }, // OP 0xB9
        { tsxInstruction, "TSX", Implied }, // OP 0xBA
        { unimplemented, "UNI", Unknown }, //LASInstruction_a_y, // OP 0xBB
        { ldyInstruction, "LDY", AbsoluteX }, // OP 0xBC
        { ldaInstruction, "LDA", AbsoluteX }, // OP 0xBD
        { ldxInstruction, "LDX", AbsoluteY }, // OP 0xBE
        { laxInstruction, "LAX", AbsoluteY }, // OP 0xBF
        { cpyInstruction, "CPY", Immediate }, // OP 0xC0
        { cmpInstruction, "CMP", IndirectX }, // OP 0xC1
        { nopInstruction, "NOP", Immediate }, // OP 0xC2
        { dcpInstruction, "DCP", IndirectX }, // OP 0xC3
        { cpyInstruction, "CPY", ZeroPage }, // OP 0xC4
        { cmpInstruction, "CMP", ZeroPage }, // OP 0xC5
        { decInstruction, "DEC", ZeroPage }, // OP 0xC6
        { dcpInstruction, "DCP", ZeroPage }, // OP 0xC7
        { inyInstruction, "INY", Implied }, // OP 0xC8
        { cmpInstruction, "CMP", Immediate }, // OP 0xC9
        { dexInstruction, "DEX", Implied }, // OP 0xCA
        { axsInstruction, "AXS", Immediate }, // OP 0xCB
        { cpyInstruction, "CPY", Absolute }, // OP 0xCC
        { cmpInstruction, "CMP", Absolute }, // OP 0xCD
        { decInstruction, "DEC", Absolute }, // OP 0xCE
        { dcpInstruction, "DCP", Absolute }, // OP 0xCF
        { bxxInstruction, "BNE", Relative }, // OP 0xD0
        { cmpInstruction, "CMP", IndirectY }, // OP 0xD1
        { stpInstruction, "STP", Implied }, // OP 0xD2
        { dcpInstruction, "DCP", IndirectY }, // OP 0xD3
        { nopInstruction, "NOP", ZeroPageX }, // OP 0xD4
        { cmpInstruction, "CMP", ZeroPageX }, // OP 0xD5
        { decInstruction, "DEC", ZeroPageX }, // OP 0xD6
        { dcpInstruction, "DCP", ZeroPageX }, // OP 0xD7
        { scxInstruction, "CLD", Implied }, // OP 0xD8
        { cmpInstruction, "CMP", AbsoluteY }, // OP 0xD9
        { nopInstruction, "NOP", Implied }, // OP 0xDA
        { dcpInstruction, "DCP", AbsoluteY }, // OP 0xDB
        { nopInstruction, "NOP", AbsoluteX }, // OP 0xDC
        { cmpInstruction, "CMP", AbsoluteX }, // OP 0xDD
        { decInstruction, "DEC", AbsoluteX }, // OP 0xDE
        { dcpInstruction, "DCP", AbsoluteX }, // OP 0xDF
        { cpxInstruction, "CPX", Immediate }, // OP 0xE0
        { sbcInstruction, "SBC", IndirectX }, // OP 0xE1
        { nopInstruction, "NOP", Immediate }, // OP 0xE2
        { isbInstruction, "ISB", IndirectX }, // OP 0xE3
        { cpxInstruction, "CPX", ZeroPage }, // OP 0xE4
        { sbcInstruction, "SBC", ZeroPage }, // OP 0xE5
        { incInstruction, "INC", ZeroPage }, // OP 0xE6
        { isbInstruction, "ISB", ZeroPage }, // OP 0xE7
        { inxInstruction, "INX", Implied }, // OP 0xE8
        { sbcInstruction, "SBC", Immediate }, // OP 0xE9
        { nopInstruction, "NOP", Implied }, // OP 0xEA
        { sbcInstruction, "SBC", Immediate }, // OP 0xEB
        { cpxInstruction, "CPX", Absolute }, // OP 0xEC
        { sbcInstruction, "SBC", Absolute }, // OP 0xED
        { incInstruction, "INC", Absolute }, // OP 0xEE
        { isbInstruction, "ISB", Absolute }, // OP 0xEF
        { bxxInstruction, "BEQ", Relative }, // OP 0xF0
        { sbcInstruction, "SBC", IndirectY }, // OP 0xF1
        { stpInstruction, "STP", Implied }, // OP 0xF2
        { isbInstruction, "ISB", IndirectY }, // OP 0xF3
        { nopInstruction, "NOP", ZeroPageX }, // OP 0xF4
        { sbcInstruction, "SBC", ZeroPageX }, // OP 0xF5
        { incInstruction, "INC", ZeroPageX }, // OP 0xF6
        { isbInstruction, "ISB", ZeroPageX }, // OP 0xF7
        { scxInstruction, "SED", Implied }, // OP 0xF8
        { sbcInstruction, "SBC", AbsoluteY }, // OP 0xF9
        { nopInstruction, "NOP", Implied }, // OP 0xFA
        { isbInstruction, "ISB", AbsoluteY }, // OP 0xFB
        { nopInstruction, "NOP", AbsoluteX }, // OP 0xFC
        { sbcInstruction, "SBC", AbsoluteX }, // OP 0xFD
        { incInstruction, "INC", AbsoluteX }, // OP 0xFE
        { isbInstruction, "ISB", AbsoluteX }, // OP 0xFF
    };

    int addressModeLengths[] = { 1, 2, 2, 2, 2, 3, 3, 3, 2, 2, 3, 2, 1 };
    string addressModeNames[] = {
        "Implied",
        "Immediate",
        "ZeroPage",
        "ZeroPageX",
        "ZeroPageY",
        "Absolute",
        "AbsoluteX",
        "AbsoluteY",
        "IndirectX",
        "IndirectY",
        "IndirectAbsolute",
        "Relative",
        "Unknown"
    };

    string addressModeShorts[] = {
        "", "I", "D", "DX", "DY", "A", "AX", "AY", "$X", "$Y", "$", "PD", "U"
    };
}