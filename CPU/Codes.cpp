//
// Created by Taylor Whatley on 2018-09-19.
//

#include "CPU.h"
#include "Operations.h"
#include "Codes.h"

#include <iostream>

namespace Nem {
    int Unimplemented(CPU *cpu) {
        std::cout << "Instruction not implemented. PC: $" << makeHex(cpu->registers->programCounter)
                  << " INST: $" << makeHex(cpu->memory->getByte(cpu->registers->programCounter)) << std::endl;

        return 0;
    }

    int NOPInstruction(CPU *) { return 0; }
    int NOPInstruction_i(CPU *) { return 1; }
    int NOPInstruction_d(CPU *) { return 1; }
    int NOPInstruction_d_x(CPU *) { return 1; }
    int NOPInstruction_a(CPU *) { return 2; }
    int NOPInstruction_a_x(CPU *) { return 2; }

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
        Byte value = cpu->memory->getByte(pointer);
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
        Address pointer = (Address) cpu->nextByte() + cpu->registers->indexX;
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

    int ISCInstruction_d(CPU *cpu) {
        Address pointer = (Address) cpu->nextByte();
        INC(cpu, pointer);
        SBC(cpu, cpu->memory->getByte(pointer));
        return 1;
    }

    int ISCInstruction_d_x(CPU *cpu) {
        Address pointer = onPage((Address) cpu->nextByte() + cpu->registers->indexX);
        INC(cpu, pointer);
        SBC(cpu, cpu->memory->getByte(pointer));
        return 1;
    }

    int ISCInstruction_a(CPU *cpu) {
        Address pointer = (Address) cpu->nextAddress();
        INC(cpu, pointer);
        SBC(cpu, cpu->memory->getByte(pointer));
        return 2;
    }

    int ISCInstruction_a_x(CPU *cpu) {
        Address pointer = (Address) cpu->nextAddress() + cpu->registers->indexX;
        INC(cpu, pointer);
        SBC(cpu, cpu->memory->getByte(pointer));
        return 2;
    }

    int ISCInstruction_a_y(CPU *cpu) {
        Address pointer = (Address) cpu->nextAddress() + cpu->registers->indexY;
        INC(cpu, pointer);
        SBC(cpu, cpu->memory->getByte(pointer));
        return 2;
    }

    int ISCInstruction_$x(CPU *cpu) {
        Address pointer = getAddressOnPage(cpu, (Address) cpu->nextAddress() + cpu->registers->indexX);
        INC(cpu, pointer);
        SBC(cpu, cpu->memory->getByte(pointer));
        return 1;
    }

    int ISCInstruction_$y(CPU *cpu) {
        Address pointer = getAddressOnPage(cpu, (Address) cpu->nextAddress()) + cpu->registers->indexY;
        INC(cpu, pointer);
        SBC(cpu, cpu->memory->getByte(pointer));
        return 1;
    }

    int DCPInstruction_d(CPU *cpu) {
        Address pointer = (Address) cpu->nextByte();
        DEC(cpu, pointer);
        CMP(cpu, cpu->registers->accumulator, cpu->memory->getByte(pointer));
        return 1;
    }

    int DCPInstruction_d_x(CPU *cpu) {
        Address pointer = onPage((Address) cpu->nextByte() + cpu->registers->indexX);
        DEC(cpu, pointer);
        CMP(cpu, cpu->registers->accumulator, cpu->memory->getByte(pointer));
        return 1;
    }

    int DCPInstruction_a(CPU *cpu) {
        Address pointer = cpu->nextAddress();
        DEC(cpu, pointer);
        CMP(cpu, cpu->registers->accumulator, cpu->memory->getByte(pointer));
        return 2;
    }

    int DCPInstruction_a_x(CPU *cpu) {
        Address pointer = cpu->nextAddress() + cpu->registers->indexX;
        DEC(cpu, pointer);
        CMP(cpu, cpu->registers->accumulator, cpu->memory->getByte(pointer));
        return 2;
    }

    int DCPInstruction_a_y(CPU *cpu) {
        Address pointer = cpu->nextAddress() + cpu->registers->indexY;
        DEC(cpu, pointer);
        CMP(cpu, cpu->registers->accumulator, cpu->memory->getByte(pointer));
        return 2;
    }

    int DCPInstruction_$x(CPU *cpu) {
        Address pointer = getAddressOnPage(cpu, (Address) cpu->nextByte() + cpu->registers->indexX);
        DEC(cpu, pointer);
        CMP(cpu, cpu->registers->accumulator, cpu->memory->getByte(pointer));
        return 1;
    }

    int DCPInstruction_$y(CPU *cpu) {
        Address pointer = getAddressOnPage(cpu, (Address) cpu->nextByte()) + cpu->registers->indexY;
        DEC(cpu, pointer);
        CMP(cpu, cpu->registers->accumulator, cpu->memory->getByte(pointer));
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
        Address pointer = (Address) cpu->nextByte() + cpu->registers->indexX;
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
        return 1;
    }

    int RORInstruction_d_x(CPU *cpu) {
        Address pointer = (Address) cpu->nextByte() + cpu->registers->indexX;
        ROR(cpu, pointer);
        return 1;
    }

    int RORInstruction_a(CPU *cpu) {
        Address pointer = cpu->nextAddress();
        ROR(cpu, pointer);
        return 2;
    }

    int RORInstruction_a_x(CPU *cpu) {
        Address pointer = cpu->nextAddress() + cpu->registers->indexX;
        ROR(cpu, pointer);
        return 2;
    }

    int SLOInstruction_d(CPU *cpu) {
        Address pointer = (Address) cpu->nextByte();
        ASL(cpu, pointer);
        ORA(cpu, cpu->memory->getByte(pointer));
        return 1;
    }

    int SLOInstruction_d_x(CPU *cpu) {
        Address pointer = onPage((Address) cpu->nextByte() + cpu->registers->indexX);
        ASL(cpu, pointer);
        ORA(cpu, cpu->memory->getByte(pointer));
        return 1;
    }

    int SLOInstruction_a(CPU *cpu) {
        Address pointer = cpu->nextAddress();
        ASL(cpu, pointer);
        ORA(cpu, cpu->memory->getByte(pointer));
        return 2;
    }

    int SLOInstruction_a_x(CPU *cpu) {
        Address pointer = cpu->nextAddress() + cpu->registers->indexX;
        ASL(cpu, pointer);
        ORA(cpu, cpu->memory->getByte(pointer));
        return 2;
    }

    int SLOInstruction_a_y(CPU *cpu) {
        Address pointer = cpu->nextAddress() + cpu->registers->indexY;
        ASL(cpu, pointer);
        ORA(cpu, cpu->memory->getByte(pointer));
        return 2;
    }

    int SLOInstruction_$x(CPU *cpu) {
        Address pointer = getAddressOnPage(cpu, (Address) cpu->nextByte() + cpu->registers->indexX);
        ASL(cpu, pointer);
        ORA(cpu, cpu->memory->getByte(pointer));
        return 1;
    }

    int SLOInstruction_$y(CPU *cpu) {
        Address pointer = getAddressOnPage(cpu, (Address) cpu->nextByte()) + cpu->registers->indexY;
        ASL(cpu, pointer);
        ORA(cpu, cpu->memory->getByte(pointer));
        return 1;
    }

    int RLAInstruction_d(CPU *cpu) {
        Address pointer = (Address) cpu->nextByte();
        ROL(cpu, pointer);
        AND(cpu, cpu->memory->getByte(pointer));
        return 1;
    }

    int RLAInstruction_d_x(CPU *cpu) {
        Address pointer = onPage((Address) cpu->nextByte() + cpu->registers->indexX);
        ROL(cpu, pointer);
        AND(cpu, cpu->memory->getByte(pointer));
        return 1;
    }

    int RLAInstruction_a(CPU *cpu) {
        Address pointer = cpu->nextAddress();
        ROL(cpu, pointer);
        AND(cpu, cpu->memory->getByte(pointer));
        return 2;
    }

    int RLAInstruction_a_x(CPU *cpu) {
        Address pointer = cpu->nextAddress() + cpu->registers->indexX;
        ROL(cpu, pointer);
        AND(cpu, cpu->memory->getByte(pointer));
        return 2;
    }

    int RLAInstruction_a_y(CPU *cpu) {
        Address pointer = cpu->nextAddress() + cpu->registers->indexY;
        ROL(cpu, pointer);
        AND(cpu, cpu->memory->getByte(pointer));
        return 2;
    }

    int RLAInstruction_$x(CPU *cpu) {
        Address pointer = getAddressOnPage(cpu, (Address) cpu->nextByte() + cpu->registers->indexX);
        ROL(cpu, pointer);
        AND(cpu, cpu->memory->getByte(pointer));
        return 1;
    }

    int RLAInstruction_$y(CPU *cpu) {
        Address pointer = getAddressOnPage(cpu, (Address) cpu->nextByte()) + cpu->registers->indexY;
        ROL(cpu, pointer);
        AND(cpu, cpu->memory->getByte(pointer));
        return 1;
    }

    int RRAInstruction_d(CPU *cpu) {
        Address pointer = (Address) cpu->nextByte();
        ROR(cpu, pointer);
        ADC(cpu, cpu->memory->getByte(pointer));
        return 1;
    }

    int RRAInstruction_d_x(CPU *cpu) {
        Address pointer = onPage((Address) cpu->nextByte() + cpu->registers->indexX);
        ROR(cpu, pointer);
        ADC(cpu, cpu->memory->getByte(pointer));
        return 1;
    }

    int RRAInstruction_a(CPU *cpu) {
        Address pointer = cpu->nextAddress();
        ROR(cpu, pointer);
        ADC(cpu, cpu->memory->getByte(pointer));
        return 2;
    }

    int RRAInstruction_a_x(CPU *cpu) {
        Address pointer = cpu->nextAddress() + cpu->registers->indexX;
        ROR(cpu, pointer);
        ADC(cpu, cpu->memory->getByte(pointer));
        return 2;
    }

    int RRAInstruction_a_y(CPU *cpu) {
        Address pointer = cpu->nextAddress() + cpu->registers->indexY;
        ROR(cpu, pointer);
        ADC(cpu, cpu->memory->getByte(pointer));
        return 2;
    }

    int RRAInstruction_$x(CPU *cpu) {
        Address pointer = getAddressOnPage(cpu, (Address) cpu->nextByte() + cpu->registers->indexX);
        ROR(cpu, pointer);
        ADC(cpu, cpu->memory->getByte(pointer));
        return 1;
    }

    int RRAInstruction_$y(CPU *cpu) {
        Address pointer = getAddressOnPage(cpu, (Address) cpu->nextByte()) + cpu->registers->indexY;
        ROR(cpu, pointer);
        ADC(cpu, cpu->memory->getByte(pointer));
        return 1;
    }

    int SREInstruction_d(CPU *cpu) {
        Address pointer = (Address) cpu->nextByte();
        LSR(cpu, pointer);
        EOR(cpu, cpu->memory->getByte(pointer));
        return 1;
    }

    int SREInstruction_d_x(CPU *cpu) {
        Address pointer = onPage((Address) cpu->nextByte() + cpu->registers->indexX);
        LSR(cpu, pointer);
        EOR(cpu, cpu->memory->getByte(pointer));
        return 1;
    }

    int SREInstruction_a(CPU *cpu) {
        Address pointer = cpu->nextAddress();
        LSR(cpu, pointer);
        EOR(cpu, cpu->memory->getByte(pointer));
        return 2;
    }

    int SREInstruction_a_x(CPU *cpu) {
        Address pointer = cpu->nextAddress() + cpu->registers->indexX;
        LSR(cpu, pointer);
        EOR(cpu, cpu->memory->getByte(pointer));
        return 2;
    }

    int SREInstruction_a_y(CPU *cpu) {
        Address pointer = cpu->nextAddress() + cpu->registers->indexY;
        LSR(cpu, pointer);
        EOR(cpu, cpu->memory->getByte(pointer));
        return 2;
    }

    int SREInstruction_$x(CPU *cpu) {
        Address pointer = getAddressOnPage(cpu, (Address) cpu->nextByte() + cpu->registers->indexX);
        LSR(cpu, pointer);
        EOR(cpu, cpu->memory->getByte(pointer));
        return 1;
    }

    int SREInstruction_$y(CPU *cpu) {
        Address pointer = getAddressOnPage(cpu, (Address) cpu->nextByte()) + cpu->registers->indexY;
        LSR(cpu, pointer);
        EOR(cpu, cpu->memory->getByte(pointer));
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
            if (skippedPage(cpu->registers->programCounter, value)) cpu->cycles += 1;
            cpu->registers->programCounter += value;
            cpu->cycles += 1;
        }
        return 1;
    }

    int BMIInstruction_pd(CPU *cpu) {
        if (cpu->isFlagSet(CPURegisters::StatusFlags::Negative)) {
            short value = makeSigned(cpu->nextByte());
            if (skippedPage(cpu->registers->programCounter, value)) cpu->cycles += 1;
            cpu->registers->programCounter += value;
            cpu->cycles += 1;
        }
        return 1;
    }

    int BVCInstruction_pd(CPU *cpu) {
        if (!cpu->isFlagSet(CPURegisters::StatusFlags::Overflow)) {
            short value = makeSigned(cpu->nextByte());
            if (skippedPage(cpu->registers->programCounter, value)) cpu->cycles += 1;
            cpu->registers->programCounter += value;
            cpu->cycles += 1;
        }
        return 1;
    }

    int BVSInstruction_pd(CPU *cpu) {
        if (cpu->isFlagSet(CPURegisters::StatusFlags::Overflow)) {
            short value = makeSigned(cpu->nextByte());
            if (skippedPage(cpu->registers->programCounter, value)) cpu->cycles += 1;
            cpu->registers->programCounter += value;
            cpu->cycles += 1;
        }
        return 1;
    }

    int BCCInstruction_pd(CPU *cpu) {
        if (!cpu->isFlagSet(CPURegisters::StatusFlags::Carry)) {
            short value = makeSigned(cpu->nextByte());
            if (skippedPage(cpu->registers->programCounter, value)) cpu->cycles += 1;
            cpu->registers->programCounter += value;
            cpu->cycles += 1;
        }
        return 1;
    }

    int BCSInstruction_pd(CPU *cpu) {
        if (cpu->isFlagSet(CPURegisters::StatusFlags::Carry)) {
            short value = makeSigned(cpu->nextByte());
            if (skippedPage(cpu->registers->programCounter, value)) cpu->cycles += 1;
            cpu->registers->programCounter += value;
            cpu->cycles += 1;
        }
        return 1;
    }

    int BNEInstruction_pd(CPU *cpu) {
        if (!cpu->isFlagSet(CPURegisters::StatusFlags::Zero)) {
            short value = makeSigned(cpu->nextByte());
            if (skippedPage(cpu->registers->programCounter, value)) cpu->cycles += 1;
            cpu->registers->programCounter += value;
            cpu->cycles += 1;
        }
        return 1;
    }

    int BEQInstruction_pd(CPU *cpu) {
        if (cpu->isFlagSet(CPURegisters::StatusFlags::Zero)) {
            short value = makeSigned(cpu->nextByte());
            if (skippedPage(cpu->registers->programCounter, value)) cpu->cycles += 1;
            cpu->registers->programCounter += value;
            cpu->cycles += 1;
        }
        return 1;
    }

    int BRKInstruction(CPU *cpu) {
        std::cout << "Break executed. PC: 0x" << makeHex(cpu->registers->programCounter) << std::endl;

        return 0;
    }

    int RTIInstruction(CPU *cpu) {
        cpu->registers->status = (Byte) (cpu->popByte() | 0b00100000);
        cpu->popAddress();
        return 0;
    }

    int JSRInstruction_a(CPU *cpu) {
        cpu->pushAddress(cpu->registers->programCounter + (Address) 2);
        cpu->registers->programCounter = cpu->nextAddress() - (Address) 1;
        return 0;
    }

    int RTSInstruction(CPU *cpu) {
        Address address = cpu->popAddress();
        cpu->registers->programCounter = address;
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
        if (skippedPage(pointer, cpu->registers->indexX)) cpu->cycles += 1;
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
        return 1;
    }

    int LAXInstruction_d_y(CPU *cpu) {
        Byte value = getByteOnPage(cpu, (Address) cpu->nextByte() + cpu->registers->indexY);
        cpu->registers->accumulator = value;
        cpu->registers->indexX = value;
        checkLow(cpu);
        return 1;
    }

    int LAXInstruction_a(CPU *cpu) {
        Byte value = cpu->memory->getByte(cpu->nextAddress());
        cpu->registers->accumulator = value;
        cpu->registers->indexX = value;
        checkLow(cpu);
        return 2;
    }

    int LAXInstruction_a_y(CPU *cpu) {
        Byte value = cpu->memory->getByte(cpu->nextAddress() + cpu->registers->indexY);
        cpu->registers->accumulator = value;
        cpu->registers->indexX = value;
        checkLow(cpu);
        return 2;
    }

    int LAXInstruction_$x(CPU *cpu) {
        Byte value = cpu->memory->getByte(getAddressOnPage(cpu, (Address) cpu->nextByte() + cpu->registers->indexX));
        cpu->registers->accumulator = value;
        cpu->registers->indexX = value;
        checkLow(cpu);
        return 1;
    }

    int LAXInstruction_$y(CPU *cpu) {
        Byte value = cpu->memory->getByte(getAddressOnPage(cpu, (Address) cpu->nextByte()) + cpu->registers->indexY);
        cpu->registers->accumulator = value;
        cpu->registers->indexX = value;
        checkLow(cpu);
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
        cpu->cycles += 6;
        return 1;
    }

    int STAInstruction_$y(CPU *cpu) {
        cpu->memory->setByte(getAddressOnPage(cpu,
                                              (Address) cpu->nextByte()) + cpu->registers->indexY,
                             cpu->registers->accumulator);
        cpu->cycles += 6;
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
        return 1;
    }

    int SAXInstruction_d_y(CPU *cpu) {
        cpu->memory->setByte(onPage((Address) cpu->nextByte() + cpu->registers->indexY),
                             cpu->registers->accumulator & cpu->registers->indexX);
        return 1;
    }

    int SAXInstruction_a(CPU *cpu) {

        cpu->memory->setByte((Address) cpu->nextAddress(),
                             cpu->registers->accumulator & cpu->registers->indexX);
        return 2;
    }

    int SAXInstruction_$x(CPU *cpu) {
        cpu->memory->setByte(cpu->memory->getAddress(onPage((Address) cpu->nextByte() + cpu->registers->indexX)),
                             cpu->registers->accumulator & cpu->registers->indexX);
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
        return 0;
    }

    int PLAInstruction(CPU *cpu) {
        cpu->registers->accumulator = cpu->popByte();
        checkLow(cpu);
        return 0;
    }

    int PHPInstruction(CPU *cpu) {
        cpu->pushByte((Byte) (cpu->registers->status | 0b00110000));
        return 0;
    }

    int PLPInstruction(CPU *cpu) {
        cpu->registers->status = (Byte) ((cpu->popByte() & ~0b00010000) | 0b00100000);
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
            Unimplemented, //ANCInstruction_i, // OP 0x0B
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
            Unimplemented, //ANCInstruction_i, // OP 0x2B
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
            Unimplemented, //ALRInstruction_i, // OP 0x4B
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
            Unimplemented, //ARRInstruction_i, // OP 0x6B
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
            Unimplemented, //AXSInstruction_i, // OP 0xCB
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
            ISCInstruction_$x, // OP 0xE3
            CPXInstruction_d, // OP 0xE4
            SBCInstruction_d, // OP 0xE5
            INCInstruction_d, // OP 0xE6
            ISCInstruction_d, // OP 0xE7
            INXInstruction, // OP 0xE8
            SBCInstruction_i, // OP 0xE9
            NOPInstruction, // OP 0xEA
            SBCInstruction_i, // OP 0xEB
            CPXInstruction_a, // OP 0xEC
            SBCInstruction_a, // OP 0xED
            INCInstruction_a, // OP 0xEE
            ISCInstruction_a, //ISCInstruction_a, // OP 0xEF
            BEQInstruction_pd, // OP 0xF0
            SBCInstruction_$y, // OP 0xF1
            Unimplemented, //STPInstruction, // OP 0xF2
            ISCInstruction_$y, // OP 0xF3
            NOPInstruction_d_x, // OP 0xF4
            SBCInstruction_d_x, // OP 0xF5
            INCInstruction_d_x, // OP 0xF6
            ISCInstruction_d_x, // OP 0xF7
            SEDInstruction, // OP 0xF8
            SBCInstruction_a_y, // OP 0xF9
            NOPInstruction, // OP 0xFA
            ISCInstruction_a_y, // OP 0xFB
            NOPInstruction_a_x, // OP 0xFC
            SBCInstruction_a_x, // OP 0xFD
            INCInstruction_a_x, // OP 0xFE
            ISCInstruction_a_x, // OP 0xFF
    };

    string opNames[] = {
            "BRK", // NAME 0x00
            "ORA $X", // NAME 0x01
            "UNI", //STPInstruction, // NAME 0x02
            "SLO $X", // NAME 0x03
            "NOP D", // NAME 0x04
            "ORA D", // NAME 0x05
            "ASL D", // NAME 0x06
            "SLO D", // NAME 0x07
            "PHP", // NAME 0x08
            "ORA I", // NAME 0x09
            "ASL", // NAME 0x0A
            "UNI", //ANCInstruction_i, // NAME 0x0B
            "NOP A", // NAME 0x0C
            "ORA A", // NAME 0x0D
            "ASL A", // NAME 0x0E
            "SLO A", //SLOInstruction_a, // NAME 0x0F
            "BPL PD", // NAME 0x10
            "ORA $Y", // NAME 0x11
            "UNI", //STPInstruction, // NAME 0x12
            "SLO $Y", // NAME 0x13
            "NOP DX", // NAME 0x14
            "ORA DX", // NAME 0x15
            "ASL DX", // NAME 0x16
            "SLO DX", // NAME 0x17
            "CLC", // NAME 0x18
            "ORA AY", // NAME 0x19
            "NOP", // NAME 0x1A
            "SLO AY", // NAME 0x1B
            "NOP AX", // NAME 0x1C
            "ORA AX", // NAME 0x1D
            "ASL AX", // NAME 0x1E
            "SLO AX", // NAME 0x1F
            "JSR A", // NAME 0x20
            "AND $X", // NAME 0x21
            "UNI", //STPInstruction, // NAME 0x22
            "RLA $X", // NAME 0x23
            "BIT D", // NAME 0x24
            "AND D", // NAME 0x25
            "ROL D", // NAME 0x26
            "RLA D", // NAME 0x27
            "PLP", // NAME 0x28
            "AND I", // NAME 0x29
            "ROL", // NAME 0x2A
            "UNI", //ANCInstruction_i, // NAME 0x2B
            "BIT A", // NAME 0x2C
            "AND A", // NAME 0x2D
            "ROL A", // NAME 0x2E
            "RLA A", // NAME 0x2F
            "BMI PD", // NAME 0x30
            "AND $Y", // NAME 0x31
            "UNI", //STPInstruction, // NAME 0x32
            "RLA $Y", // NAME 0x33
            "NOP DX", // NAME 0x34
            "AND DX", // NAME 0x35
            "ROL DX", // NAME 0x36
            "RLA DX", // NAME 0x37
            "SEC", // NAME 0x38
            "AND AY", // NAME 0x39
            "NOP", // NAME 0x3A
            "RLA AY", // NAME 0x3B
            "NOP AX", // NAME 0x3C
            "AND AX", // NAME 0x3D
            "ROL AX", // NAME 0x3E
            "RLA AX", // NAME 0x3F
            "RTI", // NAME 0x40
            "EOR $X", // NAME 0x41
            "UNI", //STPInstruction, // NAME 0x42
            "SRE $X", // NAME 0x43
            "NOP D", // NAME 0x44
            "EOR D", // NAME 0x45
            "LSR D", // NAME 0x46
            "SRE D", // NAME 0x47
            "PHA", // NAME 0x48
            "EOR I", // NAME 0x49
            "LSR", // NAME 0x4A
            "UNI", //ALRInstruction_i, // NAME 0x4B
            "JMP A", // NAME 0x4C
            "EOR A", // NAME 0x4D
            "LSR A", // NAME 0x4E
            "SRE A", // NAME 0x4F
            "BVC PD", // NAME 0x50
            "EOR $Y", // NAME 0x51
            "UNI", //STPInstruction, // NAME 0x52
            "SRE $Y", // NAME 0x53
            "NOP DX", // NAME 0x54
            "EOR DX", // NAME 0x55
            "LSR DX", // NAME 0x56
            "SRE DX", // NAME 0x57
            "CLI", // NAME 0x58
            "EOR AY", // NAME 0x59
            "NOP", // NAME 0x5A
            "SRE AY", // NAME 0x5B
            "NOP AX", // NAME 0x5C
            "EOR AX", // NAME 0x5D
            "LSR AX", // NAME 0x5E
            "SRE AX", // NAME 0x5F
            "RTS", // NAME 0x60
            "ADC $X", // NAME 0x61
            "UNI", //STPInstruction, // NAME 0x62
            "RRA $X", // NAME 0x63
            "NOP D", // NAME 0x64
            "ADC D", // NAME 0x65
            "ROR D", // NAME 0x66
            "RRA D", // NAME 0x67
            "PLA", // NAME 0x68
            "ADC I", // NAME 0x69
            "ROR", // NAME 0x6A
            "UNI", //ARRInstruction_i, // NAME 0x6B
            "JMP $", // NAME 0x6C
            "ADC A", // NAME 0x6D
            "ROR A", // NAME 0x6E
            "RRA A", // NAME 0x6F
            "BVS PD", // NAME 0x70
            "ADC $Y", // NAME 0x71
            "UNI", //STPInstruction, // NAME 0x72
            "RRA $Y", // NAME 0x73
            "NOP DX", // NAME 0x74
            "ADC DX", // NAME 0x75
            "ROR DX", // NAME 0x76
            "RRA DX", // NAME 0x77
            "SEI", // NAME 0x78
            "ADC AY", // NAME 0x79
            "NOP", // NAME 0x7A
            "RRA AY", // NAME 0x7B
            "NOP AX", // NAME 0x7C
            "ADC AX", // NAME 0x7D
            "ROR AX", // NAME 0x7E
            "RRA AX", // NAME 0x7F
            "NOP I", // NAME 0x80
            "STA $X", // NAME 0x81
            "NOP I", // NAME 0x82
            "SAX $X", // NAME 0x83
            "STY D", // NAME 0x84
            "STA D", // NAME 0x85
            "STX D", // NAME 0x86
            "SAX D", // NAME 0x87
            "DEY", // NAME 0x88
            "NOP I", // NAME 0x89
            "TXA", // NAME 0x8A
            "UNI", //XAAInstruction_i, // NAME 0x8B
            "STY A", // NAME 0x8C
            "STA A", // NAME 0x8D
            "STX A", // NAME 0x8E
            "SAX A", // NAME 0x8F
            "BCC PD", // NAME 0x90
            "STA $Y", // NAME 0x91
            "UNI", //STPInstruction, // NAME 0x92
            "UNI", //AHXInstruction_d_y, // NAME 0x93
            "STY DX", // NAME 0x94
            "STA DX", // NAME 0x95
            "STX DY", // NAME 0x96
            "SAX DY", // NAME 0x97
            "TYA", // NAME 0x98
            "STA AY", // NAME 0x99
            "TXS", // NAME 0x9A
            "UNI", //TASInstruction_a_y, // NAME 0x9B
            "UNI", //SHYInstruction_a_x, // NAME 0x9C
            "STA AX", // NAME 0x9D
            "UNI", //SHXInstruction_a_y, // NAME 0x9E
            "UNI", //AHXInstruction_a_y, // NAME 0x9F
            "LDY I", // NAME 0xA0
            "LDA $X", // NAME 0xA1
            "LDX I", // NAME 0xA2
            "LAX $X", // NAME 0xA3
            "LDY D", // NAME 0xA4
            "LDA D", // NAME 0xA5
            "LDX D", // NAME 0xA6
            "LAX D", // NAME 0xA7
            "TAY", // NAME 0xA8
            "LDA I", // NAME 0xA9
            "TAX", // NAME 0xAA
            "LAX I", // NAME 0xAB
            "LDY A", // NAME 0xAC
            "LDA A", // NAME 0xAD
            "LDX A", // NAME 0xAE
            "LAX A", // NAME 0xAF
            "BCS PD", // NAME 0xB0
            "LDA $Y", // NAME 0xB1
            "UNI", //STPInstruction, // NAME 0xB2
            "LAX $Y", // NAME 0xB3
            "LDY DX", // NAME 0xB4
            "LDA DX", // NAME 0xB5
            "LDX DY", // NAME 0xB6
            "LAX DY", // NAME 0xB7
            "CLV", // NAME 0xB8
            "LDA AY", // NAME 0xB9
            "TSX", // NAME 0xBA
            "UNI", //LASInstruction_a_y, // NAME 0xBB
            "LDY AX", // NAME 0xBC
            "LDA AX", // NAME 0xBD
            "LDX AY", // NAME 0xBE
            "LAX AY", // NAME 0xBF
            "CPY I", // NAME 0xC0
            "CMP $X", // NAME 0xC1
            "NOP I", // NAME 0xC2
            "DCP $X", // NAME 0xC3
            "CPY D", // NAME 0xC4
            "CMP D", // NAME 0xC5
            "DEC D", // NAME 0xC6
            "DCP D", // NAME 0xC7
            "INY", // NAME 0xC8
            "CMP I", // NAME 0xC9
            "DEX", // NAME 0xCA
            "UNI", //AXSInstruction_i, // NAME 0xCB
            "CPY A", // NAME 0xCC
            "CMP A", // NAME 0xCD
            "DEC A", // NAME 0xCE
            "DCP A", // NAME 0xCF
            "BNE PD", // NAME 0xD0
            "CMP $Y", // NAME 0xD1
            "UNI", //STPInstruction, // NAME 0xD2
            "DCP $Y", // NAME 0xD3
            "NOP DX", // NAME 0xD4
            "CMP DX", // NAME 0xD5
            "DEC DX", // NAME 0xD6
            "DCP DX", // NAME 0xD7
            "CLD", // NAME 0xD8
            "CMP AY", // NAME 0xD9
            "NOP", // NAME 0xDA
            "DCP AY", // NAME 0xDB
            "NOP AX", // NAME 0xDC
            "CMP AX", // NAME 0xDD
            "DEC AX", // NAME 0xDE
            "DCP AX", // NAME 0xDF
            "CPX I", // NAME 0xE0
            "SBC $X", // NAME 0xE1
            "NOP I", // NAME 0xE2
            "ISB $X", // NAME 0xE3
            "CPX D", // NAME 0xE4
            "SBC D", // NAME 0xE5
            "INC D", // NAME 0xE6
            "ISB D", // NAME 0xE7
            "INX", // NAME 0xE8
            "SBC I", // NAME 0xE9
            "NOP", // NAME 0xEA
            "SBC I", // NAME 0xEB
            "CPX A", // NAME 0xEC
            "SBC A", // NAME 0xED
            "INC A", // NAME 0xEE
            "ISB A", // NAME 0xEF
            "BEQ PD", // NAME 0xF0
            "SBC $Y", // NAME 0xF1
            "UNI", //STPInstruction, // NAME 0xF2
            "ISB $y", // NAME 0xF3
            "NOP DX", // NAME 0xF4
            "SBC DX", // NAME 0xF5
            "INC DX", // NAME 0xF6
            "ISB DX", // NAME 0xF7
            "SED", // NAME 0xF8
            "SBC AY", // NAME 0xF9
            "NOP", // NAME 0xFA
            "ISB AY", // NAME 0xFB
            "NOP AX", // NAME 0xFC
            "SBC AX", // NAME 0xFD
            "INC AX", // NAME 0xFE
            "ISB AX", // NAME 0xFF
    };
}