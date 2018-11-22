//
// Created by Taylor Whatley on 2018-10-17.
//

#ifndef NEM_DISASSEMBLER_H
#define NEM_DISASSEMBLER_H

#include "../Internal.h"

#include "../CPU/CPU.h"
#include "../CPU/CPUCodes.h"

namespace Nem {
    class InstInfo {
    public:
        string name;
        Byte code;
        int length;

        AddressMode mode;

        virtual string toString() const;

        explicit InstInfo(Byte inst);
        virtual ~InstInfo() = default;
    };

    class DisInst : public InstInfo {
    public:
        InstArguments arguments;
        CPURegisters registers;
        int cycles;

        string toString() const override;
        vector<Byte> toBinary() const;

        DisInst(Byte memory[3], CPURegisters nRegisters, int nCycles);
    };
}

#endif //NEM_DISASSEMBLER_H
