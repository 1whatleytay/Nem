//
// Created by Taylor Whatley on 2018-10-17.
//

#include "Disassembler.h"

#include "../CPU/CPU.h"

#include <sstream>
#include <iostream>

namespace Nem {
    string opNames[] = {
            "BRK", // NAME 0x00
            "ORA", // NAME 0x01
            "UNI", //STPInstruction, // NAME 0x02
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
            "UNI", //STPInstruction, // NAME 0x12
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
            "UNI", //STPInstruction, // NAME 0x22
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
            "UNI", //STPInstruction, // NAME 0x32
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
            "UNI", //STPInstruction, // NAME 0x42
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
            "UNI", //STPInstruction, // NAME 0x52
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
            "UNI", //STPInstruction, // NAME 0x62
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
            "UNI", //STPInstruction, // NAME 0x72
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
            "UNI", //STPInstruction, // NAME 0x92
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
            "UNI", //STPInstruction, // NAME 0xB2
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
            "UNI", //STPInstruction, // NAME 0xD2
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
            "UNI", //STPInstruction, // NAME 0xF2
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
            Unknown, // MODE 0x02
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
            Unknown, // MODE 0x12
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
            Absolute, // MODE 0x20
            IndirectX, // MODE 0x21
            Unknown, // MODE 0x22
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
            Unknown, // MODE 0x32
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
            Absolute, // MODE 0x4C
            Absolute, // MODE 0x4D
            Absolute, // MODE 0x4E
            Absolute, // MODE 0x4F
            Relative, // MODE 0x50
            IndirectY, // MODE 0x51
            Unknown, // MODE 0x52
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
            Unknown, // MODE 0x62
            IndirectX, // MODE 0x63
            ZeroPage, // MODE 0x64
            ZeroPage, // MODE 0x65
            ZeroPage, // MODE 0x66
            ZeroPage, // MODE 0x67
            Implied, // MODE 0x68
            Immediate, // MODE 0x69
            Implied, // MODE 0x6A
            Immediate, // MODE 0x6B
            IndirectAbsolute, // MODE 0x6C
            Absolute, // MODE 0x6D
            Absolute, // MODE 0x6E
            Absolute, // MODE 0x6F
            Relative, // MODE 0x70
            IndirectY, // MODE 0x71
            Unknown, // MODE 0x72
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
            IndirectX, // MODE 0x81
            Immediate, // MODE 0x82
            IndirectX, // MODE 0x83
            ZeroPage, // MODE 0x84
            ZeroPage, // MODE 0x85
            ZeroPage, // MODE 0x86
            ZeroPage, // MODE 0x87
            Implied, // MODE 0x88
            Immediate, // MODE 0x89
            Implied, // MODE 0x8A
            Unknown, // MODE 0x8B
            Absolute, // MODE 0x8C
            Absolute, // MODE 0x8D
            Absolute, // MODE 0x8E
            Absolute, // MODE 0x8F
            Relative, // MODE 0x90
            IndirectY, // MODE 0x91
            Unknown, // MODE 0x92
            Unknown, // MODE 0x93
            ZeroPageX, // MODE 0x94
            ZeroPageX, // MODE 0x95
            ZeroPageY, // MODE 0x96
            ZeroPageY, // MODE 0x97
            Implied, // MODE 0x98
            AbsoluteY, // MODE 0x99
            Implied, // MODE 0x9A
            Unknown, // MODE 0x9B
            Unknown, // MODE 0x9C
            AbsoluteX, // MODE 0x9D
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
            Unknown, // MODE 0xB2
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
            Unknown, // MODE 0xD2
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
            Unknown, // MODE 0xF2
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

    int getAddressModeLength(AddressMode mode) {
        switch (mode) {
            case Implied: return 1;
            case Immediate: return 2;
            case ZeroPage: return 2;
            case ZeroPageX: return 2;
            case ZeroPageY: return 2;
            case Absolute: return 3;
            case AbsoluteX: return 3;
            case AbsoluteY: return 3;
            case IndirectX: return 2;
            case IndirectY: return 2;
            case IndirectAbsolute: return 3;
            case Relative: return 2;
            case Unknown: return 1;
        }
    }

    string addSpace(string in) { return in.empty() ? in : " " + in; }

    string getAddressModeShort(AddressMode mode) {
        switch (mode) {
            case Implied: return "";
            case Immediate: return "I";
            case ZeroPage: return "D";
            case ZeroPageX: return "DX";
            case ZeroPageY: return "DY";
            case Absolute: return "A";
            case AbsoluteX: return "AX";
            case AbsoluteY: return "AY";
            case IndirectX: return "$X";
            case IndirectY: return "$Y";
            case IndirectAbsolute: return "$";
            case Relative: return "PD";
            case Unknown: return "";
        }
    }

    string formatArguments(InstArguments arguments, AddressMode mode, Address pc) {
        switch (mode) {
            case Implied: break;
            case Immediate: return "#$" + makeHex(arguments.value);
            case ZeroPage: return "$" + makeHex(arguments.value);
            case ZeroPageX: return "$" + makeHex(arguments.value) + ", X";
            case ZeroPageY: return "$" + makeHex(arguments.value) + ", Y";
            case Absolute: return "$" + makeHex(arguments.pointer);
            case AbsoluteX: return "$" + makeHex(arguments.pointer) + ", X";
            case AbsoluteY: return "$" + makeHex(arguments.pointer) + ", Y";
            case IndirectX: return "($" + makeHex(arguments.pointer) + ", X)";
            case IndirectY: return "($" + makeHex(arguments.pointer) + "), Y";
            case IndirectAbsolute: return "($" + makeHex(arguments.pointer) + ")";
            case Relative: return "$" + makeHex(pc + makeSigned(arguments.value));
            case Unknown: break;
        }
        return "";
    }

    string InstInfo::toString() const {
        std::stringstream stream;
        stream << "[" << makeHex(code) << ": " << name << addSpace(getAddressModeShort(mode)) << "]";
        return stream.str();
    }

    InstInfo::InstInfo(Byte instruction) {
        code = instruction;
        name = opNames[instruction];
        mode = opModes[instruction];
        length = getAddressModeLength(mode);
    }

    string DisInst::toString() const {
        std::stringstream stream;
        stream << "["
        << makeHex(registers.programCounter) << ": "
        << makeHex(code) << ": " << name << addSpace(formatArguments(arguments, mode, registers.programCounter))
        << " ("
        << "A: " << makeHex(registers.accumulator)
        << " X: " << makeHex(registers.indexX)
        << " Y: " << makeHex(registers.indexY)
        << " P: " << makeHex(registers.status)
        << " SP: " << makeHex(registers.stackPointer)
        << ")"
        << "]";
        return stream.str();
    }

    // 11 Bytes
    vector<Byte> DisInst::toBinary() const {
        vector<Byte> data = vector<Byte>(1 + sizeof(InstArguments) + sizeof(CPURegisters));
        data[0] = code;
        memcpy(&data[1], &arguments, sizeof(InstArguments));
        memcpy(&data[1 + sizeof(InstArguments)], &registers, sizeof(CPURegisters));
        return data;
    }

    DisInst::DisInst(Byte fetch[3], CPURegisters* nRegisters)
        : InstInfo(fetch[0]) {
        if (length == 2) arguments.value = fetch[1];
        else if (length == 3) arguments.pointer = makeAddress(fetch[1], fetch[2]);
        registers = *nRegisters;
    }
}