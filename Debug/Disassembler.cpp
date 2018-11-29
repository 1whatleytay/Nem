//
// Created by Taylor Whatley on 2018-10-17.
//

#include "Disassembler.h"

#include "../CPU/CPU.h"

#include <sstream>
#include <iostream>

namespace Nem {
    string addSpace(string in) { return in.empty() ? in : " " + in; }

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
            default: break;
        }
        return "";
    }

    string InstInfo::toString() const {
        std::stringstream stream;
        stream << "[" << makeHex(code) << ": " << name << addSpace(addressModeShorts[mode]) << "]";
        return stream.str();
    }

    InstInfo::InstInfo(Byte instruction) {
        code = instruction;
        name = opInstructions[instruction].name;
        mode = opInstructions[instruction].mode;
        length = addressModeLengths[mode];
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
        << " CYC: " << cycles
        << ")]";
        return stream.str();
    }

    // 19 bytes
    vector<Byte> DisInst::toBinary() const {
        vector<Byte> data = vector<Byte>(sizeof(Byte) + sizeof(InstArguments) + sizeof(CPURegisters) + sizeof(int));
        data[0] = code;
        memcpy(&data[sizeof(Byte)], &arguments, sizeof(InstArguments));
        memcpy(&data[sizeof(Byte) + sizeof(InstArguments)], &registers, sizeof(CPURegisters));
        memcpy(&data[sizeof(Byte) + sizeof(InstArguments) + sizeof(CPURegisters)], &cycles, sizeof(int));
        return data;
    }

    DisInst::DisInst(Byte fetch[3], CPURegisters nRegisters, int nCycles)
        : InstInfo(fetch[0]), registers(nRegisters), cycles(nCycles) {
        if (length == 2) arguments.value = fetch[1];
        else if (length == 3) arguments.pointer = makeAddress(fetch[1], fetch[2]);
    }
}