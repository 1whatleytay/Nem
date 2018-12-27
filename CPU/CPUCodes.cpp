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
        cpu->profiler->message(Profiler::Breakpoint);
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
        cpu->profiler->message(Profiler::Breakpoint);
#endif

        cpu->stopExec();
        return true;
    }

    bool stpInstruction(CPU* cpu, AddressMode, InstArguments) {
        std::cout << "Halt executed. PC: 0x" << makeHex(cpu->registers.programCounter) << std::endl;

#ifdef NEM_PROFILE
        cpu->profiler->message(Profiler::Breakpoint);
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
        bool read = !inst.noRead;
        int length = addressModeLengths[inst.mode];
        Byte next = 0;
        Address nextA = 0;
        if (length <= 2) next = cpu->nextByte();
        else if (length >= 3) nextA = cpu->nextAddress();
        switch (inst.mode) {
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
        return inst.function(cpu, inst.mode, arguments) ? addressModeLengths[inst.mode] : 1;
    }

    Instruction opInstructions[] = {
        { brkInstruction, "BRK", Implied, false }, // OP 0x00
        { oraInstruction, "ORA", IndirectX, false }, // OP 0x01
        { stpInstruction, "STP", Implied, false }, // OP 0x02
        { sloInstruction, "SLO", IndirectX, false }, // OP 0x03
        { nopInstruction, "NOP", ZeroPage, false }, // OP 0x04
        { oraInstruction, "ORA", ZeroPage, false }, // OP 0x05
        { aslInstruction, "ASL", ZeroPage, false }, // OP 0x06
        { sloInstruction, "SLO", ZeroPage, false }, // OP 0x07
        { phpInstruction, "PHP", Implied, false }, // OP 0x08
        { oraInstruction, "ORA", Immediate, false }, // OP 0x09
        { aslInstruction, "ASL", Implied, false }, // OP 0x0A
        { ancInstruction, "ANC", Immediate, false }, // OP 0x0B
        { nopInstruction, "NOP", Absolute, false }, // OP 0x0C
        { oraInstruction, "ORA", Absolute, false }, // OP 0x0D
        { aslInstruction, "ASL", Absolute, false }, // OP 0x0E
        { sloInstruction, "SLO", Absolute, false }, // OP 0x0F
        { bxxInstruction, "BPL", Relative, false }, // OP 0x10
        { oraInstruction, "ORA", IndirectY, false }, // OP 0x11
        { stpInstruction, "STP", Implied, false }, // OP 0x12
        { sloInstruction, "SLO", IndirectY, false }, // OP 0x13
        { nopInstruction, "NOP", ZeroPageX, false }, // OP 0x14
        { oraInstruction, "ORA", ZeroPageX, false }, // OP 0x15
        { aslInstruction, "ASL", ZeroPageX, false }, // OP 0x16
        { sloInstruction, "SLO", ZeroPageX, false }, // OP 0x17
        { scxInstruction, "CLC", Implied, false }, // OP 0x18
        { oraInstruction, "ORA", AbsoluteY, false }, // OP 0x19
        { nopInstruction, "NOP", Implied, false }, // OP 0x1A
        { sloInstruction, "SLO", AbsoluteY, false }, // OP 0x1B
        { nopInstruction, "NOP", AbsoluteX, false }, // OP 0x1C
        { oraInstruction, "ORA", AbsoluteX, false }, // OP 0x1D
        { aslInstruction, "ASL", AbsoluteX, false }, // OP 0x1E
        { sloInstruction, "SLO", AbsoluteX, false }, // OP 0x1F
        { jsrInstruction, "JSR", Absolute, true }, // OP 0x20
        { andInstruction, "AND", IndirectX, false }, // OP 0x21
        { stpInstruction, "STP", Implied, false }, // OP 0x22
        { rlaInstruction, "RLA", IndirectX, false }, // OP 0x23
        { bitInstruction, "BIT", ZeroPage, false }, // OP 0x24
        { andInstruction, "AND", ZeroPage, false }, // OP 0x25
        { rolInstruction, "ROL", ZeroPage, false }, // OP 0x26
        { rlaInstruction, "RLA", ZeroPage, false }, // OP 0x27
        { plpInstruction, "PLP", Implied, false }, // OP 0x28
        { andInstruction, "AND", Immediate, false }, // OP 0x29
        { rolInstruction, "ROL", Implied, false }, // OP 0x2A
        { ancInstruction, "ANC", Immediate, false }, // OP 0x2B
        { bitInstruction, "BIT", Absolute, false }, // OP 0x2C
        { andInstruction, "AND", Absolute, false }, // OP 0x2D
        { rolInstruction, "ROL", Absolute, false }, // OP 0x2E
        { rlaInstruction, "RLA", Absolute, false }, // OP 0x2F
        { bxxInstruction, "BMI", Relative, false }, // OP 0x30
        { andInstruction, "AND", IndirectY, false }, // OP 0x31
        { stpInstruction, "STP", Implied, false }, // OP 0x32
        { rlaInstruction, "RLA", IndirectY, false }, // OP 0x33
        { nopInstruction, "NOP", ZeroPageX, false }, // OP 0x34
        { andInstruction, "AND", ZeroPageX, false }, // OP 0x35
        { rolInstruction, "ROL", ZeroPageX, false }, // OP 0x36
        { rlaInstruction, "RLA", ZeroPageX, false }, // OP 0x37
        { scxInstruction, "SEC", Implied, false }, // OP 0x38
        { andInstruction, "AND", AbsoluteY, false }, // OP 0x39
        { nopInstruction, "NOP", Implied, false }, // OP 0x3A
        { rlaInstruction, "RLA", AbsoluteY, false }, // OP 0x3B
        { nopInstruction, "NOP", AbsoluteX, false }, // OP 0x3C
        { andInstruction, "AND", AbsoluteX, false }, // OP 0x3D
        { rolInstruction, "ROL", AbsoluteX, false }, // OP 0x3E
        { rlaInstruction, "RLA", AbsoluteX, false }, // OP 0x3F
        { rtiInstruction, "RTI", Implied, false }, // OP 0x40
        { eorInstruction, "EOR", IndirectX, false }, // OP 0x41
        { stpInstruction, "STP", Implied, false }, // OP 0x42
        { sreInstruction, "SRE", IndirectX, false }, // OP 0x43
        { nopInstruction, "NOP", ZeroPage, false }, // OP 0x44
        { eorInstruction, "EOR", ZeroPage, false }, // OP 0x45
        { lsrInstruction, "LSR", ZeroPage, false }, // OP 0x46
        { sreInstruction, "SRE", ZeroPage, false }, // OP 0x47
        { phaInstruction, "PHA", Implied, false }, // OP 0x48
        { eorInstruction, "EOR", Immediate, false }, // OP 0x49
        { lsrInstruction, "LSR", Implied, false }, // OP 0x4A
        { alrInstruction, "ALR", Immediate, false }, // OP 0x4B
        { jmpInstruction, "JMP", Absolute, true }, // OP 0x4C
        { eorInstruction, "EOR", Absolute, false }, // OP 0x4D
        { lsrInstruction, "LSR", Absolute, false }, // OP 0x4E
        { sreInstruction, "SRE", Absolute, false }, // OP 0x4F
        { bxxInstruction, "BVC", Relative, false }, // OP 0x50
        { eorInstruction, "EOR", IndirectY, false }, // OP 0x51
        { stpInstruction, "STP", Implied, false }, // OP 0x52
        { sreInstruction, "SRE", IndirectY, false }, // OP 0x53
        { nopInstruction, "NOP", ZeroPageX, false }, // OP 0x54
        { eorInstruction, "EOR", ZeroPageX, false }, // OP 0x55
        { lsrInstruction, "LSR", ZeroPageX, false }, // OP 0x56
        { sreInstruction, "SRE", ZeroPageX, false }, // OP 0x57
        { scxInstruction, "CLI", Implied, false }, // OP 0x58
        { eorInstruction, "EOR", AbsoluteY, false }, // OP 0x59
        { nopInstruction, "NOP", Implied, false }, // OP 0x5A
        { sreInstruction, "SRE", AbsoluteY, false }, // OP 0x5B
        { nopInstruction, "NOP", AbsoluteX, false }, // OP 0x5C
        { eorInstruction, "EOR", AbsoluteX, false }, // OP 0x5D
        { lsrInstruction, "LSR", AbsoluteX, false }, // OP 0x5E
        { sreInstruction, "SRE", AbsoluteX, false }, // OP 0x5F
        { rtsInstruction, "RTS", Implied, false }, // OP 0x60
        { adcInstruction, "ADC", IndirectX, false }, // OP 0x61
        { stpInstruction, "STP", Implied, false }, // OP 0x62
        { rraInstruction, "RRA", IndirectX, false }, // OP 0x63
        { nopInstruction, "NOP", ZeroPage, false }, // OP 0x64
        { adcInstruction, "ADC", ZeroPage, false }, // OP 0x65
        { rorInstruction, "ROR", ZeroPage, false }, // OP 0x66
        { rraInstruction, "RRA", ZeroPage, false }, // OP 0x67
        { plaInstruction, "PLA", Implied, false }, // OP 0x68
        { adcInstruction, "ADC", Immediate, false }, // OP 0x69
        { rorInstruction, "ROR", Implied, false }, // OP 0x6A
        { arrInstruction, "ARR", Immediate, false }, // OP 0x6B
        { jmpInstruction, "JMP", IndirectAbsolute, true }, // OP 0x6C
        { adcInstruction, "ADC", Absolute, false }, // OP 0x6D
        { rorInstruction, "ROR", Absolute, false }, // OP 0x6E
        { rraInstruction, "RRA", Absolute, false }, // OP 0x6F
        { bxxInstruction, "BVS", Relative, false }, // OP 0x70
        { adcInstruction, "ADC", IndirectY, false }, // OP 0x71
        { stpInstruction, "STP", Implied, false }, // OP 0x72
        { rraInstruction, "RRA", IndirectY, false }, // OP 0x73
        { nopInstruction, "NOP", ZeroPageX, false }, // OP 0x74
        { adcInstruction, "ADC", ZeroPageX, false }, // OP 0x75
        { rorInstruction, "ROR", ZeroPageX, false }, // OP 0x76
        { rraInstruction, "RRA", ZeroPageX, false }, // OP 0x77
        { scxInstruction, "SEI", Implied, false }, // OP 0x78
        { adcInstruction, "ADC", AbsoluteY, false }, // OP 0x79
        { nopInstruction, "NOP", Implied, false }, // OP 0x7A
        { rraInstruction, "RRA", AbsoluteY, false }, // OP 0x7B
        { nopInstruction, "NOP", AbsoluteX, false }, // OP 0x7C
        { adcInstruction, "ADC", AbsoluteX, false }, // OP 0x7D
        { rorInstruction, "ROR", AbsoluteX, false }, // OP 0x7E
        { rraInstruction, "RRA", AbsoluteX, false }, // OP 0x7F
        { nopInstruction, "NOP", Immediate, false }, // OP 0x80
        { staInstruction, "STA", IndirectX, true }, // OP 0x81
        { nopInstruction, "NOP", Immediate, false }, // OP 0x82
        { saxInstruction, "SAX", IndirectX, true }, // OP 0x83
        { styInstruction, "STY", ZeroPage, true }, // OP 0x84
        { staInstruction, "STA", ZeroPage, true }, // OP 0x85
        { stxInstruction, "STX", ZeroPage, true }, // OP 0x86
        { saxInstruction, "SAX", ZeroPage, true }, // OP 0x87
        { deyInstruction, "DEY", Implied, false }, // OP 0x88
        { nopInstruction, "NOP", Immediate, false }, // OP 0x89
        { txaInstruction, "TXA", Implied, false }, // OP 0x8A
        { unimplemented, "UNI", Unknown, false }, //XAAInstruction_i, // OP 0x8B
        { styInstruction, "STY", Absolute, true }, // OP 0x8C
        { staInstruction, "STA", Absolute, true }, // OP 0x8D
        { stxInstruction, "STX", Absolute, true }, // OP 0x8E
        { saxInstruction, "SAX", Absolute, true }, // OP 0x8F
        { bxxInstruction, "BCC", Relative, false }, // OP 0x90
        { staInstruction, "STA", IndirectY, true }, // OP 0x91
        { stpInstruction, "STP", Implied, false }, // OP 0x92
        { unimplemented, "UNI", Unknown, false }, //AHXInstruction_d_y, // OP 0x93
        { styInstruction, "STY", ZeroPageX, true }, // OP 0x94
        { staInstruction, "STA", ZeroPageX, true }, // OP 0x95
        { stxInstruction, "STX", ZeroPageY, true }, // OP 0x96
        { saxInstruction, "SAX", ZeroPageY, true }, // OP 0x97
        { tyaInstruction, "TYA", Implied, false }, // OP 0x98
        { staInstruction, "STA", AbsoluteY, true }, // OP 0x99
        { txsInstruction, "TXS", Implied, false }, // OP 0x9A
        { unimplemented, "UNI", Unknown, false }, //TASInstruction_a_y, // OP 0x9B
        { unimplemented, "UNI", Unknown, false }, //SHYInstruction_a_x, // OP 0x9C
        { staInstruction, "STA", AbsoluteX, true }, // OP 0x9D
        { unimplemented, "UNI", Unknown, false }, //SHXInstruction_a_y, // OP 0x9E
        { unimplemented, "UNI", Unknown, false }, //AHXInstruction_a_y, // OP 0x9F
        { ldyInstruction, "LDY", Immediate, false }, // OP 0xA0
        { ldaInstruction, "LDA", IndirectX, false }, // OP 0xA1
        { ldxInstruction, "LDX", Immediate, false }, // OP 0xA2
        { laxInstruction, "LAX", IndirectX, false }, // OP 0xA3
        { ldyInstruction, "LDY", ZeroPage, false }, // OP 0xA4
        { ldaInstruction, "LDA", ZeroPage, false }, // OP 0xA5
        { ldxInstruction, "LDX", ZeroPage, false }, // OP 0xA6
        { laxInstruction, "LAX", ZeroPage, false }, // OP 0xA7
        { tayInstruction, "TAY", Implied, false }, // OP 0xA8
        { ldaInstruction, "LDA", Immediate, false }, // OP 0xA9
        { taxInstruction, "TAX", Implied, false }, // OP 0xAA
        { laxInstruction, "LAX", Immediate, false }, // OP 0xAB
        { ldyInstruction, "LDY", Absolute, false }, // OP 0xAC
        { ldaInstruction, "LDA", Absolute, false }, // OP 0xAD
        { ldxInstruction, "LDX", Absolute, false }, // OP 0xAE
        { laxInstruction, "LAX", Absolute, false }, // OP 0xAF
        { bxxInstruction, "BCS", Relative, false }, // OP 0xB0
        { ldaInstruction, "LDA", IndirectY, false }, // OP 0xB1
        { stpInstruction, "STP", Implied, false }, // OP 0xB2
        { laxInstruction, "LAX", IndirectY, false }, // OP 0xB3
        { ldyInstruction, "LDY", ZeroPageX, false }, // OP 0xB4
        { ldaInstruction, "LDA", ZeroPageX, false }, // OP 0xB5
        { ldxInstruction, "LDX", ZeroPageY, false }, // OP 0xB6
        { laxInstruction, "LAX", ZeroPageY, false }, // OP 0xB7
        { scxInstruction, "CLV", Implied, false }, // OP 0xB8
        { ldaInstruction, "LDA", AbsoluteY, false }, // OP 0xB9
        { tsxInstruction, "TSX", Implied, false }, // OP 0xBA
        { unimplemented, "UNI", Unknown, false }, //LASInstruction_a_y, // OP 0xBB
        { ldyInstruction, "LDY", AbsoluteX, false }, // OP 0xBC
        { ldaInstruction, "LDA", AbsoluteX, false }, // OP 0xBD
        { ldxInstruction, "LDX", AbsoluteY, false }, // OP 0xBE
        { laxInstruction, "LAX", AbsoluteY, false }, // OP 0xBF
        { cpyInstruction, "CPY", Immediate, false }, // OP 0xC0
        { cmpInstruction, "CMP", IndirectX, false }, // OP 0xC1
        { nopInstruction, "NOP", Immediate, false }, // OP 0xC2
        { dcpInstruction, "DCP", IndirectX, false }, // OP 0xC3
        { cpyInstruction, "CPY", ZeroPage, false }, // OP 0xC4
        { cmpInstruction, "CMP", ZeroPage, false }, // OP 0xC5
        { decInstruction, "DEC", ZeroPage, false }, // OP 0xC6
        { dcpInstruction, "DCP", ZeroPage, false }, // OP 0xC7
        { inyInstruction, "INY", Implied, false }, // OP 0xC8
        { cmpInstruction, "CMP", Immediate, false }, // OP 0xC9
        { dexInstruction, "DEX", Implied, false }, // OP 0xCA
        { axsInstruction, "AXS", Immediate, false }, // OP 0xCB
        { cpyInstruction, "CPY", Absolute, false }, // OP 0xCC
        { cmpInstruction, "CMP", Absolute, false }, // OP 0xCD
        { decInstruction, "DEC", Absolute, false }, // OP 0xCE
        { dcpInstruction, "DCP", Absolute, false }, // OP 0xCF
        { bxxInstruction, "BNE", Relative, false }, // OP 0xD0
        { cmpInstruction, "CMP", IndirectY, false }, // OP 0xD1
        { stpInstruction, "STP", Implied, false }, // OP 0xD2
        { dcpInstruction, "DCP", IndirectY, false }, // OP 0xD3
        { nopInstruction, "NOP", ZeroPageX, false }, // OP 0xD4
        { cmpInstruction, "CMP", ZeroPageX, false }, // OP 0xD5
        { decInstruction, "DEC", ZeroPageX, false }, // OP 0xD6
        { dcpInstruction, "DCP", ZeroPageX, false }, // OP 0xD7
        { scxInstruction, "CLD", Implied, false }, // OP 0xD8
        { cmpInstruction, "CMP", AbsoluteY, false }, // OP 0xD9
        { nopInstruction, "NOP", Implied, false }, // OP 0xDA
        { dcpInstruction, "DCP", AbsoluteY, false }, // OP 0xDB
        { nopInstruction, "NOP", AbsoluteX, false }, // OP 0xDC
        { cmpInstruction, "CMP", AbsoluteX, false }, // OP 0xDD
        { decInstruction, "DEC", AbsoluteX, false }, // OP 0xDE
        { dcpInstruction, "DCP", AbsoluteX, false }, // OP 0xDF
        { cpxInstruction, "CPX", Immediate, false }, // OP 0xE0
        { sbcInstruction, "SBC", IndirectX, false }, // OP 0xE1
        { nopInstruction, "NOP", Immediate, false }, // OP 0xE2
        { isbInstruction, "ISB", IndirectX, false }, // OP 0xE3
        { cpxInstruction, "CPX", ZeroPage, false }, // OP 0xE4
        { sbcInstruction, "SBC", ZeroPage, false }, // OP 0xE5
        { incInstruction, "INC", ZeroPage, false }, // OP 0xE6
        { isbInstruction, "ISB", ZeroPage, false }, // OP 0xE7
        { inxInstruction, "INX", Implied, false }, // OP 0xE8
        { sbcInstruction, "SBC", Immediate, false }, // OP 0xE9
        { nopInstruction, "NOP", Implied, false }, // OP 0xEA
        { sbcInstruction, "SBC", Immediate, false }, // OP 0xEB
        { cpxInstruction, "CPX", Absolute, false }, // OP 0xEC
        { sbcInstruction, "SBC", Absolute, false }, // OP 0xED
        { incInstruction, "INC", Absolute, false }, // OP 0xEE
        { isbInstruction, "ISB", Absolute, false }, // OP 0xEF
        { bxxInstruction, "BEQ", Relative, false }, // OP 0xF0
        { sbcInstruction, "SBC", IndirectY, false }, // OP 0xF1
        { stpInstruction, "STP", Implied, false }, // OP 0xF2
        { isbInstruction, "ISB", IndirectY, false }, // OP 0xF3
        { nopInstruction, "NOP", ZeroPageX, false }, // OP 0xF4
        { sbcInstruction, "SBC", ZeroPageX, false }, // OP 0xF5
        { incInstruction, "INC", ZeroPageX, false }, // OP 0xF6
        { isbInstruction, "ISB", ZeroPageX, false }, // OP 0xF7
        { scxInstruction, "SED", Implied, false }, // OP 0xF8
        { sbcInstruction, "SBC", AbsoluteY, false }, // OP 0xF9
        { nopInstruction, "NOP", Implied, false }, // OP 0xFA
        { isbInstruction, "ISB", AbsoluteY, false }, // OP 0xFB
        { nopInstruction, "NOP", AbsoluteX, false }, // OP 0xFC
        { sbcInstruction, "SBC", AbsoluteX, false }, // OP 0xFD
        { incInstruction, "INC", AbsoluteX, false }, // OP 0xFE
        { isbInstruction, "ISB", AbsoluteX, false }, // OP 0xFF
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