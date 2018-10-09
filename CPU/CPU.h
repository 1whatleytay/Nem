//
// Created by Taylor Whatley on 2018-09-28.
//

#ifndef NEM_CPU_H
#define NEM_CPU_H

#include "../Internal.h"

#ifdef PRINT_INSTRUCTIONS
#include <fstream>
#endif

namespace Nem {
    class ROM;
    class PPU;
    class Clock;

    enum CPUMemoryRegion {
        WorkRam      = 0x0000,
        PPUIO = 0x2000,
        APUIO  = 0x4000,
        SRAM         = 0x4020,
        PRGRom       = 0x8000,
    };

    class CPUMemory {
        PPU* ppu = nullptr;
        ROM* rom = nullptr;

        vector<Byte> workRam = vector<Byte>(kilobyte(2));
    public:
        struct MappedAddress {
            CPUMemoryRegion region;
            Address effectiveAddress;
        };

        bool lockStack = true;

        void setPPU(PPU* nPPU);

        void list(Address start, Address count);

        MappedAddress mapAddress(Address address);

        Byte getByte(Address address);
        Address getAddress(Address address);
        void setByte(Address address, Byte value);
        void setAddress(Address address, Address value);

        Address getNMIVector();
        Address getResetVector();
        Address getIRQVector();

        explicit CPUMemory(ROM* nRom);
    };

    class CPURegisters {
    public:
        enum StatusFlags {
            Carry             = 0b00000001,
            Zero              = 0b00000010,
            Interrupt         = 0b00000100,
            Decimal           = 0b00001000,
            BreakCommand      = 0b00110000,
            Overflow          = 0b01000000,
            Negative          = 0b10000000,
        };

        Address programCounter = 0;
        Byte stackPointer = 0xFD;
        Byte accumulator = 0;
        Byte indexX = 0, indexY = 0;
        Byte status = 0x24;
    };

    class CPU {
        Clock* masterClock;

        volatile bool stopExecution = false;

        void processIRQ();
        void processNMI();
    public:
        long long cycles = 0;
        bool irq = false, nmi = false;

        CPUMemory* memory = nullptr;
        CPURegisters* registers = nullptr;

#ifdef PRINT_INSTRUCTIONS
        std::ofstream outFile = std::ofstream("/Users/desgroup/Desktop/nem.log.txt");
#endif

#ifdef MARIO_8057
        void wait8057();
#endif

        Byte nextByte(Address next = 1);
        Address nextAddress(Address next = 1);

        bool isFlagSet(CPURegisters::StatusFlags flags);
        void clearFlags(CPURegisters::StatusFlags flags);
        void setFlags(CPURegisters::StatusFlags flags);
        void setFlags(bool condition, Nem::CPURegisters::StatusFlags flags);

        void pushByte(Byte byte);
        void pushAddress(Address address);
        Byte popByte();
        Address popAddress();

        void setPPU(PPU* nPPU);

        void step();
        void exec();
        void stopExec();

        CPU(Clock* nMasterClock, ROM* nROM);
        ~CPU();
    };
}

#endif //NEM_CPU_H
