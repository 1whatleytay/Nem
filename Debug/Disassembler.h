//
// Created by Taylor Whatley on 2018-10-17.
//

#ifndef NEM_DISASSEMBLER_H
#define NEM_DISASSEMBLER_H

#include "../Internal.h"

namespace Nem {
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
    };

    struct DisassembledInstruction {
        string name;
        Byte code;
        int instructionLength;

        AddressMode addressMode;

        union {
            Byte value;
            Address pointer;
        };
    };

    DisassembledInstruction disassemble(Byte* memory, Address index) {
        return DisassembledInstruction();
    }
}

#endif //NEM_DISASSEMBLER_H
