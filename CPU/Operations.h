//
// Created by Taylor Whatley on 2018-09-24.
//

#ifndef NEM_OPERATIONS_H
#define NEM_OPERATIONS_H

#include "../Internal.h"

class CPU;

namespace Nem {
    bool isNegative(Byte value);

    inline bool afterPage(Address a, Address b) {
        return hi(a) < hi(b);
    }

    inline bool skippedPage(Address pointer, short offset) {
        return pointer / 256 < (pointer + offset) / 256;
    }

    inline bool skippedPage(Address pointer, Byte offset) {
        return skippedPage(pointer, (short)offset);
    }

    void checkZero(CPU* cpu, Byte value);
    void checkZero(CPU* cpu);

    void checkNegative(CPU* cpu, Byte value);
    void checkNegative(CPU* cpu);

    void checkLow(CPU* cpu, Byte value);
    void checkLow(CPU* cpu);

    Address onPage(Address offset, Byte page = 0);

    Byte getByteOnPage(CPU* cpu, Address offset, Byte page = 0);
    Address getAddressOnPage(CPU* cpu, Address offset, Byte page = 0);

    void ADC(CPU* cpu, Byte value);
    void SBC(CPU* cpu, Byte value);
    void INC(CPU* cpu, Address pointer);
    void DEC(CPU* cpu, Address pointer);

    void AND(CPU* cpu, Byte value);
    void ORA(CPU* cpu, Byte value);
    void EOR(CPU* cpu, Byte value);
    void ASL(CPU* cpu);
    void ASL(CPU* cpu, Address pointer);
    void LSR(CPU* cpu);
    void LSR(CPU* cpu, Address pointer);
    void ROL(CPU* cpu);
    void ROL(CPU* cpu, Address pointer);
    void ROR(CPU* cpu);
    void ROR(CPU* cpu, Address pointer);
    void BIT(CPU* cpu, Byte value);

    void CMP(CPU* cpu, Byte reg, Byte value);

    void JMP(CPU* cpu, Address pointer);
}

#endif //NEM_OPERATIONS_H
