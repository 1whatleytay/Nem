//
// Created by Taylor Whatley on 2018-09-19.
//

#ifndef NEM_OPCODES_H
#define NEM_OPCODES_H

#include "../Internal.h"

#include <functional>

namespace Nem {
    class CPU;

    struct InstArguments {
        Byte value;
        Address pointer;
        bool skipped;
    };

    enum AddressMode {
        Implied,
        Immediate,
        ZeroPage,
        ZeroPageX,
        ZeroPageY,
        Absolute,
        AbsoluteX,
        AbsoluteY,
        IndirectX,
        IndirectY,
        IndirectAbsolute,
        Relative,
        Unknown,
        AddressModeCount,
        NoRead = 0b10000000,
    };

    typedef std::function<bool(CPU* cpu, AddressMode mode, InstArguments arguments)> AddressedInstruction;
    struct Instruction {
        AddressedInstruction function;
        string name;
        AddressMode mode;
    };

    int callInstruction(Instruction &inst, CPU *cpu);

    extern Instruction opInstructions[256];

    extern int addressModeLengths[AddressModeCount];
    extern string addressModeNames[AddressModeCount];
    extern string addressModeShorts[AddressModeCount];
}

#endif //NEM_OPCODES_H
