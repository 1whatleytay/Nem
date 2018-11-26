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
        arguments.faster = true;
        incInstruction(cpu, mode, arguments);
        arguments.value = cpu->memory.getByte(arguments.pointer, false);
        sbcInstruction(cpu, mode, arguments);
        return true;
    }

    bool dcpInstruction(CPU* cpu, AddressMode mode, InstArguments arguments) {
        arguments.faster = true;
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
        arguments.faster = true;
        aslInstruction(cpu, mode, arguments);
        arguments.value = cpu->memory.getByte(arguments.pointer, false);
        oraInstruction(cpu, mode, arguments);
        return true;
    }

    bool sreInstruction(CPU* cpu, AddressMode mode, InstArguments arguments) {
        arguments.faster = true;
        lsrInstruction(cpu, mode, arguments);
        arguments.value = cpu->memory.getByte(arguments.pointer, false);
        eorInstruction(cpu, mode, arguments);
        return true;
    }

    bool rlaInstruction(CPU* cpu, AddressMode mode, InstArguments arguments) {
        arguments.faster = true;
        rolInstruction(cpu, mode, arguments);
        arguments.value = cpu->memory.getByte(arguments.pointer, false);
        andInstruction(cpu, mode, arguments);
        return true;
    }

    bool rraInstruction(CPU* cpu, AddressMode mode, InstArguments arguments) {
        arguments.faster = true;
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

    int callAddressMode(AddressedInstruction& inst, AddressMode mode, CPU* cpu) {
        InstArguments arguments = { 0, 0, false, false };
        bool read = (mode & NoRead) != NoRead;
        mode = (AddressMode)(mode & 0b00001111);
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
                arguments.pointer = onPage((Address)next + cpu->readCycle(cpu->registers.indexX));
                if (read) arguments.value = cpu->memory.getByte(arguments.pointer);
                break;
            case ZeroPageY:
                arguments.pointer = onPage((Address)next + cpu->readCycle(cpu->registers.indexY));
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
        return inst(cpu, mode, arguments) ? addressModeLengths[mode] : 1;
    }

    AddressedInstruction opInstructions[] = {
        brkInstruction, // OP 0x00
        oraInstruction, // OP 0x01
        stpInstruction, // OP 0x02
        sloInstruction, // OP 0x03
        nopInstruction, // OP 0x04
        oraInstruction, // OP 0x05
        aslInstruction, // OP 0x06
        sloInstruction, // OP 0x07
        phpInstruction, // OP 0x08
        oraInstruction, // OP 0x09
        aslInstruction, // OP 0x0A
        ancInstruction, // OP 0x0B
        nopInstruction, // OP 0x0C
        oraInstruction, // OP 0x0D
        aslInstruction, // OP 0x0E
        sloInstruction, // OP 0x0F
        bxxInstruction, // OP 0x10
        oraInstruction, // OP 0x11
        stpInstruction, // OP 0x12
        sloInstruction, // OP 0x13
        nopInstruction, // OP 0x14
        oraInstruction, // OP 0x15
        aslInstruction, // OP 0x16
        sloInstruction, // OP 0x17
        scxInstruction, // OP 0x18
        oraInstruction, // OP 0x19
        nopInstruction, // OP 0x1A
        sloInstruction, // OP 0x1B
        nopInstruction, // OP 0x1C
        oraInstruction, // OP 0x1D
        aslInstruction, // OP 0x1E
        sloInstruction, // OP 0x1F
        jsrInstruction, // OP 0x20
        andInstruction, // OP 0x21
        stpInstruction, // OP 0x22
        rlaInstruction, // OP 0x23
        bitInstruction, // OP 0x24
        andInstruction, // OP 0x25
        rolInstruction, // OP 0x26
        rlaInstruction, // OP 0x27
        plpInstruction, // OP 0x28
        andInstruction, // OP 0x29
        rolInstruction, // OP 0x2A
        ancInstruction, // OP 0x2B
        bitInstruction, // OP 0x2C
        andInstruction, // OP 0x2D
        rolInstruction, // OP 0x2E
        rlaInstruction, // OP 0x2F
        bxxInstruction, // OP 0x30
        andInstruction, // OP 0x31
        stpInstruction, // OP 0x32
        rlaInstruction, // OP 0x33
        nopInstruction, // OP 0x34
        andInstruction, // OP 0x35
        rolInstruction, // OP 0x36
        rlaInstruction, // OP 0x37
        scxInstruction, // OP 0x38
        andInstruction, // OP 0x39
        nopInstruction, // OP 0x3A
        rlaInstruction, // OP 0x3B
        nopInstruction, // OP 0x3C
        andInstruction, // OP 0x3D
        rolInstruction, // OP 0x3E
        rlaInstruction, // OP 0x3F
        rtiInstruction, // OP 0x40
        eorInstruction, // OP 0x41
        stpInstruction, // OP 0x42
        sreInstruction, // OP 0x43
        nopInstruction, // OP 0x44
        eorInstruction, // OP 0x45
        lsrInstruction, // OP 0x46
        sreInstruction, // OP 0x47
        phaInstruction, // OP 0x48
        eorInstruction, // OP 0x49
        lsrInstruction, // OP 0x4A
        alrInstruction, // OP 0x4B
        jmpInstruction, // OP 0x4C
        eorInstruction, // OP 0x4D
        lsrInstruction, // OP 0x4E
        sreInstruction, // OP 0x4F
        bxxInstruction, // OP 0x50
        eorInstruction, // OP 0x51
        stpInstruction, // OP 0x52
        sreInstruction, // OP 0x53
        nopInstruction, // OP 0x54
        eorInstruction, // OP 0x55
        lsrInstruction, // OP 0x56
        sreInstruction, // OP 0x57
        scxInstruction, // OP 0x58
        eorInstruction, // OP 0x59
        nopInstruction, // OP 0x5A
        sreInstruction, // OP 0x5B
        nopInstruction, // OP 0x5C
        eorInstruction, // OP 0x5D
        lsrInstruction, // OP 0x5E
        sreInstruction, // OP 0x5F
        rtsInstruction, // OP 0x60
        adcInstruction, // OP 0x61
        stpInstruction, // OP 0x62
        rraInstruction, // OP 0x63
        nopInstruction, // OP 0x64
        adcInstruction, // OP 0x65
        rorInstruction, // OP 0x66
        rraInstruction, // OP 0x67
        plaInstruction, // OP 0x68
        adcInstruction, // OP 0x69
        rorInstruction, // OP 0x6A
        arrInstruction, // OP 0x6B
        jmpInstruction, // OP 0x6C
        adcInstruction, // OP 0x6D
        rorInstruction, // OP 0x6E
        rraInstruction, // OP 0x6F
        bxxInstruction, // OP 0x70
        adcInstruction, // OP 0x71
        stpInstruction, // OP 0x72
        rraInstruction, // OP 0x73
        nopInstruction, // OP 0x74
        adcInstruction, // OP 0x75
        rorInstruction, // OP 0x76
        rraInstruction, // OP 0x77
        scxInstruction, // OP 0x78
        adcInstruction, // OP 0x79
        nopInstruction, // OP 0x7A
        rraInstruction, // OP 0x7B
        nopInstruction, // OP 0x7C
        adcInstruction, // OP 0x7D
        rorInstruction, // OP 0x7E
        rraInstruction, // OP 0x7F
        nopInstruction, // OP 0x80
        staInstruction, // OP 0x81
        nopInstruction, // OP 0x82
        saxInstruction, // OP 0x83
        styInstruction, // OP 0x84
        staInstruction, // OP 0x85
        stxInstruction, // OP 0x86
        saxInstruction, // OP 0x87
        deyInstruction, // OP 0x88
        nopInstruction, // OP 0x89
        txaInstruction, // OP 0x8A
        unimplemented, //XAAInstruction_i, // OP 0x8B
        styInstruction, // OP 0x8C
        staInstruction, // OP 0x8D
        stxInstruction, // OP 0x8E
        saxInstruction, // OP 0x8F
        bxxInstruction, // OP 0x90
        staInstruction, // OP 0x91
        stpInstruction, // OP 0x92
        unimplemented, //AHXInstruction_d_y, // OP 0x93
        styInstruction, // OP 0x94
        staInstruction, // OP 0x95
        stxInstruction, // OP 0x96
        saxInstruction, // OP 0x97
        tyaInstruction, // OP 0x98
        staInstruction, // OP 0x99
        txsInstruction, // OP 0x9A
        unimplemented, //TASInstruction_a_y, // OP 0x9B
        unimplemented, //SHYInstruction_a_x, // OP 0x9C
        staInstruction, // OP 0x9D
        unimplemented, //SHXInstruction_a_y, // OP 0x9E
        unimplemented, //AHXInstruction_a_y, // OP 0x9F
        ldyInstruction, // OP 0xA0
        ldaInstruction, // OP 0xA1
        ldxInstruction, // OP 0xA2
        laxInstruction, // OP 0xA3
        ldyInstruction, // OP 0xA4
        ldaInstruction, // OP 0xA5
        ldxInstruction, // OP 0xA6
        laxInstruction, // OP 0xA7
        tayInstruction, // OP 0xA8
        ldaInstruction, // OP 0xA9
        taxInstruction, // OP 0xAA
        laxInstruction, // OP 0xAB
        ldyInstruction, // OP 0xAC
        ldaInstruction, // OP 0xAD
        ldxInstruction, // OP 0xAE
        laxInstruction, // OP 0xAF
        bxxInstruction, // OP 0xB0
        ldaInstruction, // OP 0xB1
        stpInstruction, // OP 0xB2
        laxInstruction, // OP 0xB3
        ldyInstruction, // OP 0xB4
        ldaInstruction, // OP 0xB5
        ldxInstruction, // OP 0xB6
        laxInstruction, // OP 0xB7
        scxInstruction, // OP 0xB8
        ldaInstruction, // OP 0xB9
        tsxInstruction, // OP 0xBA
        unimplemented, //LASInstruction_a_y, // OP 0xBB
        ldyInstruction, // OP 0xBC
        ldaInstruction, // OP 0xBD
        ldxInstruction, // OP 0xBE
        laxInstruction, // OP 0xBF
        cpyInstruction, // OP 0xC0
        cmpInstruction, // OP 0xC1
        nopInstruction, // OP 0xC2
        dcpInstruction, // OP 0xC3
        cpyInstruction, // OP 0xC4
        cmpInstruction, // OP 0xC5
        decInstruction, // OP 0xC6
        dcpInstruction, // OP 0xC7
        inyInstruction, // OP 0xC8
        cmpInstruction, // OP 0xC9
        dexInstruction, // OP 0xCA
        axsInstruction, // OP 0xCB
        cpyInstruction, // OP 0xCC
        cmpInstruction, // OP 0xCD
        decInstruction, // OP 0xCE
        dcpInstruction, // OP 0xCF
        bxxInstruction, // OP 0xD0
        cmpInstruction, // OP 0xD1
        stpInstruction, // OP 0xD2
        dcpInstruction, // OP 0xD3
        nopInstruction, // OP 0xD4
        cmpInstruction, // OP 0xD5
        decInstruction, // OP 0xD6
        dcpInstruction, // OP 0xD7
        scxInstruction, // OP 0xD8
        cmpInstruction, // OP 0xD9
        nopInstruction, // OP 0xDA
        dcpInstruction, // OP 0xDB
        nopInstruction, // OP 0xDC
        cmpInstruction, // OP 0xDD
        decInstruction, // OP 0xDE
        dcpInstruction, // OP 0xDF
        cpxInstruction, // OP 0xE0
        sbcInstruction, // OP 0xE1
        nopInstruction, // OP 0xE2
        isbInstruction, // OP 0xE3
        cpxInstruction, // OP 0xE4
        sbcInstruction, // OP 0xE5
        incInstruction, // OP 0xE6
        isbInstruction, // OP 0xE7
        inxInstruction, // OP 0xE8
        sbcInstruction, // OP 0xE9
        nopInstruction, // OP 0xEA
        sbcInstruction, // OP 0xEB
        cpxInstruction, // OP 0xEC
        sbcInstruction, // OP 0xED
        incInstruction, // OP 0xEE
        isbInstruction, // OP 0xEF
        bxxInstruction, // OP 0xF0
        sbcInstruction, // OP 0xF1
        stpInstruction, // OP 0xF2
        isbInstruction, // OP 0xF3
        nopInstruction, // OP 0xF4
        sbcInstruction, // OP 0xF5
        incInstruction, // OP 0xF6
        isbInstruction, // OP 0xF7
        scxInstruction, // OP 0xF8
        sbcInstruction, // OP 0xF9
        nopInstruction, // OP 0xFA
        isbInstruction, // OP 0xFB
        nopInstruction, // OP 0xFC
        sbcInstruction, // OP 0xFD
        incInstruction, // OP 0xFE
        isbInstruction, // OP 0xFF
    };

    string opNames[] = {
        "BRK", // NAME 0x00
        "ORA", // NAME 0x01
        "STP", // NAME 0x02
        "SLO", // NAME 0x03
        "NOP", // NAME 0x04
        "ORA", // NAME 0x05
        "ASL", // NAME 0x06
        "SLO", // NAME 0x07
        "PHP", // NAME 0x08
        "ORA", // NAME 0x09
        "ASL", // NAME 0x0A
        "ANC", // NAME 0x0B
        "NOP", // NAME 0x0C
        "ORA", // NAME 0x0D
        "ASL", // NAME 0x0E
        "SLO", // NAME 0x0F
        "BPL", // NAME 0x10
        "ORA", // NAME 0x11
        "STP", // NAME 0x12
        "SLO", // NAME 0x13
        "NOP", // NAME 0x14
        "ORA", // NAME 0x15
        "ASL", // NAME 0x16
        "SLO", // NAME 0x17
        "CLC", // NAME 0x18
        "ORA", // NAME 0x19
        "NOP", // NAME 0x1A
        "SLO", // NAME 0x1B
        "NOP", // NAME 0x1C
        "ORA", // NAME 0x1D
        "ASL", // NAME 0x1E
        "SLO", // NAME 0x1F
        "JSR", // NAME 0x20
        "AND", // NAME 0x21
        "STP", // NAME 0x22
        "RLA", // NAME 0x23
        "BIT", // NAME 0x24
        "AND", // NAME 0x25
        "ROL", // NAME 0x26
        "RLA", // NAME 0x27
        "PLP", // NAME 0x28
        "AND", // NAME 0x29
        "ROL", // NAME 0x2A
        "ANC", // NAME 0x2B
        "BIT", // NAME 0x2C
        "AND", // NAME 0x2D
        "ROL", // NAME 0x2E
        "RLA", // NAME 0x2F
        "BMI", // NAME 0x30
        "AND", // NAME 0x31
        "STP", // NAME 0x32
        "RLA", // NAME 0x33
        "NOP", // NAME 0x34
        "AND", // NAME 0x35
        "ROL", // NAME 0x36
        "RLA", // NAME 0x37
        "SEC", // NAME 0x38
        "AND", // NAME 0x39
        "NOP", // NAME 0x3A
        "RLA", // NAME 0x3B
        "NOP", // NAME 0x3C
        "AND", // NAME 0x3D
        "ROL", // NAME 0x3E
        "RLA", // NAME 0x3F
        "RTI", // NAME 0x40
        "EOR", // NAME 0x41
        "STP", // NAME 0x42
        "SRE", // NAME 0x43
        "NOP", // NAME 0x44
        "EOR", // NAME 0x45
        "LSR", // NAME 0x46
        "SRE", // NAME 0x47
        "PHA", // NAME 0x48
        "EOR", // NAME 0x49
        "LSR", // NAME 0x4A
        "ALR", // NAME 0x4B
        "JMP", // NAME 0x4C
        "EOR", // NAME 0x4D
        "LSR", // NAME 0x4E
        "SRE", // NAME 0x4F
        "BVC", // NAME 0x50
        "EOR", // NAME 0x51
        "STP", // NAME 0x52
        "SRE", // NAME 0x53
        "NOP", // NAME 0x54
        "EOR", // NAME 0x55
        "LSR", // NAME 0x56
        "SRE", // NAME 0x57
        "CLI", // NAME 0x58
        "EOR", // NAME 0x59
        "NOP", // NAME 0x5A
        "SRE", // NAME 0x5B
        "NOP", // NAME 0x5C
        "EOR", // NAME 0x5D
        "LSR", // NAME 0x5E
        "SRE", // NAME 0x5F
        "RTS", // NAME 0x60
        "ADC", // NAME 0x61
        "STP", // NAME 0x62
        "RRA", // NAME 0x63
        "NOP", // NAME 0x64
        "ADC", // NAME 0x65
        "ROR", // NAME 0x66
        "RRA", // NAME 0x67
        "PLA", // NAME 0x68
        "ADC", // NAME 0x69
        "ROR", // NAME 0x6A
        "ARR", // NAME 0x6B
        "JMP", // NAME 0x6C
        "ADC", // NAME 0x6D
        "ROR", // NAME 0x6E
        "RRA", // NAME 0x6F
        "BVS", // NAME 0x70
        "ADC", // NAME 0x71
        "STP", // NAME 0x72
        "RRA", // NAME 0x73
        "NOP", // NAME 0x74
        "ADC", // NAME 0x75
        "ROR", // NAME 0x76
        "RRA", // NAME 0x77
        "SEI", // NAME 0x78
        "ADC", // NAME 0x79
        "NOP", // NAME 0x7A
        "RRA", // NAME 0x7B
        "NOP", // NAME 0x7C
        "ADC", // NAME 0x7D
        "ROR", // NAME 0x7E
        "RRA", // NAME 0x7F
        "NOP", // NAME 0x80
        "STA", // NAME 0x81
        "NOP", // NAME 0x82
        "SAX", // NAME 0x83
        "STY", // NAME 0x84
        "STA", // NAME 0x85
        "STX", // NAME 0x86
        "SAX", // NAME 0x87
        "DEY", // NAME 0x88
        "NOP", // NAME 0x89
        "TXA", // NAME 0x8A
        "UNI", //XAAInstruction_i, // NAME 0x8B
        "STY", // NAME 0x8C
        "STA", // NAME 0x8D
        "STX", // NAME 0x8E
        "SAX", // NAME 0x8F
        "BCC", // NAME 0x90
        "STA", // NAME 0x91
        "STP", // NAME 0x92
        "UNI", //AHXInstruction_d_y, // NAME 0x93
        "STY", // NAME 0x94
        "STA", // NAME 0x95
        "STX", // NAME 0x96
        "SAX", // NAME 0x97
        "TYA", // NAME 0x98
        "STA", // NAME 0x99
        "TXS", // NAME 0x9A
        "UNI", //TASInstruction_a_y, // NAME 0x9B
        "UNI", //SHYInstruction_a_x, // NAME 0x9C
        "STA", // NAME 0x9D
        "UNI", //SHXInstruction_a_y, // NAME 0x9E
        "UNI", //AHXInstruction_a_y, // NAME 0x9F
        "LDY", // NAME 0xA0
        "LDA", // NAME 0xA1
        "LDX", // NAME 0xA2
        "LAX", // NAME 0xA3
        "LDY", // NAME 0xA4
        "LDA", // NAME 0xA5
        "LDX", // NAME 0xA6
        "LAX", // NAME 0xA7
        "TAY", // NAME 0xA8
        "LDA", // NAME 0xA9
        "TAX", // NAME 0xAA
        "LAX", // NAME 0xAB
        "LDY", // NAME 0xAC
        "LDA", // NAME 0xAD
        "LDX", // NAME 0xAE
        "LAX", // NAME 0xAF
        "BCS", // NAME 0xB0
        "LDA", // NAME 0xB1
        "STP", // NAME 0xB2
        "LAX", // NAME 0xB3
        "LDY", // NAME 0xB4
        "LDA", // NAME 0xB5
        "LDX", // NAME 0xB6
        "LAX", // NAME 0xB7
        "CLV", // NAME 0xB8
        "LDA", // NAME 0xB9
        "TSX", // NAME 0xBA
        "UNI", //LASInstruction_a_y, // NAME 0xBB
        "LDY", // NAME 0xBC
        "LDA", // NAME 0xBD
        "LDX", // NAME 0xBE
        "LAX", // NAME 0xBF
        "CPY", // NAME 0xC0
        "CMP", // NAME 0xC1
        "NOP", // NAME 0xC2
        "DCP", // NAME 0xC3
        "CPY", // NAME 0xC4
        "CMP", // NAME 0xC5
        "DEC", // NAME 0xC6
        "DCP", // NAME 0xC7
        "INY", // NAME 0xC8
        "CMP", // NAME 0xC9
        "DEX", // NAME 0xCA
        "AXS", // NAME 0xCB
        "CPY", // NAME 0xCC
        "CMP", // NAME 0xCD
        "DEC", // NAME 0xCE
        "DCP", // NAME 0xCF
        "BNE", // NAME 0xD0
        "CMP", // NAME 0xD1
        "STP", // NAME 0xD2
        "DCP", // NAME 0xD3
        "NOP", // NAME 0xD4
        "CMP", // NAME 0xD5
        "DEC", // NAME 0xD6
        "DCP", // NAME 0xD7
        "CLD", // NAME 0xD8
        "CMP", // NAME 0xD9
        "NOP", // NAME 0xDA
        "DCP", // NAME 0xDB
        "NOP", // NAME 0xDC
        "CMP", // NAME 0xDD
        "DEC", // NAME 0xDE
        "DCP", // NAME 0xDF
        "CPX", // NAME 0xE0
        "SBC", // NAME 0xE1
        "NOP", // NAME 0xE2
        "ISB", // NAME 0xE3
        "CPX", // NAME 0xE4
        "SBC", // NAME 0xE5
        "INC", // NAME 0xE6
        "ISB", // NAME 0xE7
        "INX", // NAME 0xE8
        "SBC", // NAME 0xE9
        "NOP", // NAME 0xEA
        "SBC", // NAME 0xEB
        "CPX", // NAME 0xEC
        "SBC", // NAME 0xED
        "INC", // NAME 0xEE
        "ISB", // NAME 0xEF
        "BEQ", // NAME 0xF0
        "SBC", // NAME 0xF1
        "STP", // NAME 0xF2
        "ISB", // NAME 0xF3
        "NOP", // NAME 0xF4
        "SBC", // NAME 0xF5
        "INC", // NAME 0xF6
        "ISB", // NAME 0xF7
        "SED", // NAME 0xF8
        "SBC", // NAME 0xF9
        "NOP", // NAME 0xFA
        "ISB", // NAME 0xFB
        "NOP", // NAME 0xFC
        "SBC", // NAME 0xFD
        "INC", // NAME 0xFE
        "ISB", // NAME 0xFF
    };

    AddressMode opModes[] = {
        Implied, // MODE 0x00
        IndirectX, // MODE 0x01
        Implied, // MODE 0x02
        IndirectX, // MODE 0x03
        ZeroPage, // MODE 0x04
        ZeroPage, // MODE 0x05
        ZeroPage, // MODE 0x06
        ZeroPage, // MODE 0x07
        Implied, // MODE 0x08
        Immediate, // MODE 0x09
        Implied, // MODE 0x0A
        Immediate, // MODE 0x0B
        Absolute, // MODE 0x0C
        Absolute, // MODE 0x0D
        Absolute, // MODE 0x0E
        Absolute, // MODE 0x0F
        Relative, // MODE 0x10
        IndirectY, // MODE 0x11
        Implied, // MODE 0x12
        IndirectY, // MODE 0x13
        ZeroPageX, // MODE 0x14
        ZeroPageX, // MODE 0x15
        ZeroPageX, // MODE 0x16
        ZeroPageX, // MODE 0x17
        Implied, // MODE 0x18
        AbsoluteY, // MODE 0x19
        Implied, // MODE 0x1A
        AbsoluteY, // MODE 0x1B
        AbsoluteX, // MODE 0x1C
        AbsoluteX, // MODE 0x1D
        AbsoluteX, // MODE 0x1E
        AbsoluteX, // MODE 0x1F
        NO_READ(Absolute), // MODE 0x20
        IndirectX, // MODE 0x21
        Implied, // MODE 0x22
        IndirectX, // MODE 0x23
        ZeroPage, // MODE 0x24
        ZeroPage, // MODE 0x25
        ZeroPage, // MODE 0x26
        ZeroPage, // MODE 0x27
        Implied, // MODE 0x28
        Immediate, // MODE 0x29
        Implied, // MODE 0x2A
        Immediate, // MODE 0x2B
        Absolute, // MODE 0x2C
        Absolute, // MODE 0x2D
        Absolute, // MODE 0x2E
        Absolute, // MODE 0x2F
        Relative, // MODE 0x30
        IndirectY, // MODE 0x31
        Implied, // MODE 0x32
        IndirectY, // MODE 0x33
        ZeroPageX, // MODE 0x34
        ZeroPageX, // MODE 0x35
        ZeroPageX, // MODE 0x36
        ZeroPageX, // MODE 0x37
        Implied, // MODE 0x38
        AbsoluteY, // MODE 0x39
        Implied, // MODE 0x3A
        AbsoluteY, // MODE 0x3B
        AbsoluteX, // MODE 0x3C
        AbsoluteX, // MODE 0x3D
        AbsoluteX, // MODE 0x3E
        AbsoluteX, // MODE 0x3F
        Implied, // MODE 0x40
        IndirectX, // MODE 0x41
        Implied, // MODE 0x42
        IndirectX, // MODE 0x43
        ZeroPage, // MODE 0x44
        ZeroPage, // MODE 0x45
        ZeroPage, // MODE 0x46
        ZeroPage, // MODE 0x47
        Implied, // MODE 0x48
        Immediate, // MODE 0x49
        Implied, // MODE 0x4A
        Immediate, // MODE 0x4B
        NO_READ(Absolute), // MODE 0x4C
        Absolute, // MODE 0x4D
        Absolute, // MODE 0x4E
        Absolute, // MODE 0x4F
        Relative, // MODE 0x50
        IndirectY, // MODE 0x51
        Implied, // MODE 0x52
        IndirectY, // MODE 0x53
        ZeroPageX, // MODE 0x54
        ZeroPageX, // MODE 0x55
        ZeroPageX, // MODE 0x56
        ZeroPageX, // MODE 0x57
        Implied, // MODE 0x58
        AbsoluteY, // MODE 0x59
        Implied, // MODE 0x5A
        AbsoluteY, // MODE 0x5B
        AbsoluteX, // MODE 0x5C
        AbsoluteX, // MODE 0x5D
        AbsoluteX, // MODE 0x5E
        AbsoluteX, // MODE 0x5F
        Implied, // MODE 0x60
        IndirectX, // MODE 0x61
        Implied, // MODE 0x62
        IndirectX, // MODE 0x63
        ZeroPage, // MODE 0x64
        ZeroPage, // MODE 0x65
        ZeroPage, // MODE 0x66
        ZeroPage, // MODE 0x67
        Implied, // MODE 0x68
        Immediate, // MODE 0x69
        Implied, // MODE 0x6A
        Immediate, // MODE 0x6B
        NO_READ(IndirectAbsolute), // MODE 0x6C
        Absolute, // MODE 0x6D
        Absolute, // MODE 0x6E
        Absolute, // MODE 0x6F
        Relative, // MODE 0x70
        IndirectY, // MODE 0x71
        Implied, // MODE 0x72
        IndirectY, // MODE 0x73
        ZeroPageX, // MODE 0x74
        ZeroPageX, // MODE 0x75
        ZeroPageX, // MODE 0x76
        ZeroPageX, // MODE 0x77
        Implied, // MODE 0x78
        AbsoluteY, // MODE 0x79
        Implied, // MODE 0x7A
        AbsoluteY, // MODE 0x7B
        AbsoluteX, // MODE 0x7C
        AbsoluteX, // MODE 0x7D
        AbsoluteX, // MODE 0x7E
        AbsoluteX, // MODE 0x7F
        Immediate, // MODE 0x80
        NO_READ(IndirectX), // MODE 0x81
        Immediate, // MODE 0x82
        NO_READ(IndirectX), // MODE 0x83
        NO_READ(ZeroPage), // MODE 0x84
        NO_READ(ZeroPage), // MODE 0x85
        NO_READ(ZeroPage), // MODE 0x86
        NO_READ(ZeroPage), // MODE 0x87
        Implied, // MODE 0x88
        Immediate, // MODE 0x89
        Implied, // MODE 0x8A
        Unknown, // MODE 0x8B
        NO_READ(Absolute), // MODE 0x8C
        NO_READ(Absolute), // MODE 0x8D
        NO_READ(Absolute), // MODE 0x8E
        NO_READ(Absolute), // MODE 0x8F
        Relative, // MODE 0x90
        NO_READ(IndirectY), // MODE 0x91
        Implied, // MODE 0x92
        Unknown, // MODE 0x93
        NO_READ(ZeroPageX), // MODE 0x94
        NO_READ(ZeroPageX), // MODE 0x95
        NO_READ(ZeroPageY), // MODE 0x96
        NO_READ(ZeroPageY), // MODE 0x97
        Implied, // MODE 0x98
        NO_READ(AbsoluteY), // MODE 0x99
        Implied, // MODE 0x9A
        Unknown, // MODE 0x9B
        Unknown, // MODE 0x9C
        NO_READ(AbsoluteX), // MODE 0x9D
        Unknown, // MODE 0x9E
        Unknown, // MODE 0x9F
        Immediate, // MODE 0xA0
        IndirectX, // MODE 0xA1
        Immediate, // MODE 0xA2
        IndirectX, // MODE 0xA3
        ZeroPage, // MODE 0xA4
        ZeroPage, // MODE 0xA5
        ZeroPage, // MODE 0xA6
        ZeroPage, // MODE 0xA7
        Implied, // MODE 0xA8
        Immediate, // MODE 0xA9
        Implied, // MODE 0xAA
        Immediate, // MODE 0xAB
        Absolute, // MODE 0xAC
        Absolute, // MODE 0xAD
        Absolute, // MODE 0xAE
        Absolute, // MODE 0xAF
        Relative, // MODE 0xB0
        IndirectY, // MODE 0xB1
        Implied, // MODE 0xB2
        IndirectY, // MODE 0xB3
        ZeroPageX, // MODE 0xB4
        ZeroPageX, // MODE 0xB5
        ZeroPageY, // MODE 0xB6
        ZeroPageY, // MODE 0xB7
        Implied, // MODE 0xB8
        AbsoluteY, // MODE 0xB9
        Implied, // MODE 0xBA
        Unknown, // MODE 0xBB
        AbsoluteX, // MODE 0xBC
        AbsoluteX, // MODE 0xBD
        AbsoluteY, // MODE 0xBE
        AbsoluteY, // MODE 0xBF
        Immediate, // MODE 0xC0
        IndirectX, // MODE 0xC1
        Immediate, // MODE 0xC2
        IndirectX, // MODE 0xC3
        ZeroPage, // MODE 0xC4
        ZeroPage, // MODE 0xC5
        ZeroPage, // MODE 0xC6
        ZeroPage, // MODE 0xC7
        Implied, // MODE 0xC8
        Immediate, // MODE 0xC9
        Implied, // MODE 0xCA
        Immediate, // MODE 0xCB
        Absolute, // MODE 0xCC
        Absolute, // MODE 0xCD
        Absolute, // MODE 0xCE
        Absolute, // MODE 0xCF
        Relative, // MODE 0xD0
        IndirectY, // MODE 0xD1
        Implied, // MODE 0xD2
        IndirectY, // MODE 0xD3
        ZeroPageX, // MODE 0xD4
        ZeroPageX, // MODE 0xD5
        ZeroPageX, // MODE 0xD6
        ZeroPageX, // MODE 0xD7
        Implied, // MODE 0xD8
        AbsoluteY, // MODE 0xD9
        Implied, // MODE 0xDA
        AbsoluteY, // MODE 0xDB
        AbsoluteX, // MODE 0xDC
        AbsoluteX, // MODE 0xDD
        AbsoluteX, // MODE 0xDE
        AbsoluteX, // MODE 0xDF
        Immediate, // MODE 0xE0
        IndirectX, // MODE 0xE1
        Immediate, // MODE 0xE2
        IndirectX, // MODE 0xE3
        ZeroPage, // MODE 0xE4
        ZeroPage, // MODE 0xE5
        ZeroPage, // MODE 0xE6
        ZeroPage, // MODE 0xE7
        Implied, // MODE 0xE8
        Immediate, // MODE 0xE9
        Implied, // MODE 0xEA
        Immediate, // MODE 0xEB
        Absolute, // MODE 0xEC
        Absolute, // MODE 0xED
        Absolute, // MODE 0xEE
        Absolute, // MODE 0xEF
        Relative, // MODE 0xF0
        IndirectY, // MODE 0xF1
        Implied, // MODE 0xF2
        IndirectY, // MODE 0xF3
        ZeroPageX, // MODE 0xF4
        ZeroPageX, // MODE 0xF5
        ZeroPageX, // MODE 0xF6
        ZeroPageX, // MODE 0xF7
        Implied, // MODE 0xF8
        AbsoluteY, // MODE 0xF9
        Implied, // MODE 0xFA
        AbsoluteY, // MODE 0xFB
        AbsoluteX, // MODE 0xFC
        AbsoluteX, // MODE 0xFD
        AbsoluteX, // MODE 0xFE
        AbsoluteX, // MODE 0xFF
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