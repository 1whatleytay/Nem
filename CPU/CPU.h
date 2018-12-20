//
// Created by Taylor Whatley on 2018-09-28.
//

#ifndef NEM_CPU_H
#define NEM_CPU_H

//#define CPU_ENTRY 0xc000
//#define RTI_MINUS_ONE
#define FORCE_SPRITE_ZERO_HIT

#include "../Internal.h"

namespace Nem {
    class CPU;
    class PPU;
    class APU;
    class Clock;
    class Mapper;
    class ControllerInterface;

#ifdef NEM_PROFILE
    class Profiler;
#endif

    enum CPUMemoryRegion {
        WorkRam      = 0x0000,
        PPUIO        = 0x2000,
        GeneralIO    = 0x4000,
        Debug        = 0x4020,
        SRAM         = 0x6000,
        PRGRom       = 0x8000,
    };

    class CPUMemory {
        CPU* cpu = nullptr;
        PPU* ppu = nullptr;
        APU* apu = nullptr;
        Mapper* mapper = nullptr;

        Byte workRam[0x800];

        ControllerInterface* controllers[2] = { nullptr, nullptr };
    public:
        struct MappedAddress {
            CPUMemoryRegion region;
            Address effectiveAddress;
        };

        void setPPU(PPU* nPPU);
        void setAPU(APU* nAPU);
        void setController(int index, ControllerInterface* controller);

        void list(Address start, Address count);

        MappedAddress mapAddress(Address address);

        Byte getByte(Address address, bool cycle = true);
        Address getAddress(Address address, bool cycle = true);
        void setByte(Address address, Byte value, bool cycle = true);
        void setAddress(Address address, Address value, bool cycle = true);

        Address getNMIVector();
        Address getResetVector();
        Address getIRQVector();

        CPUMemory(CPU* nCpu, Mapper* nMapper);
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
        PPU* ppu = nullptr;
        Clock* clock = nullptr;
        volatile bool stopExecution = false;

        bool irq = false, nmi = false;

        void processIRQ();
        void processNMI();

        void waitCycle();
    public:
        int cycles = 0;

#ifdef NEM_PROFILE
        Profiler* profiler = nullptr;
#endif

        CPUMemory memory;
        CPURegisters registers;

        //void postIRQ();
        void postNMI();

        Byte thisByte(bool cycle = true);
        Byte nextByte(bool cycle = true);
        Address nextAddress(bool cycle = true);

        void readCycle();
        void writeCycle();

        bool isFlagSet(CPURegisters::StatusFlags flags);
        void clearFlags(CPURegisters::StatusFlags flags);
        void setFlags(CPURegisters::StatusFlags flags);
        void setFlags(bool condition, Nem::CPURegisters::StatusFlags flags);

        void pushByte(Byte byte);
        void pushAddress(Address address);
        Byte popByte();
        Address popAddress();

        void setPPU(PPU* nPPU);
        void setAPU(APU* nAPU);
        void setController(int index, ControllerInterface* controller);

        void step();
        void exec();
        void stopExec();

        CPU(Mapper* mapper, Clock* clock);
        ~CPU();
    };
}

#endif //NEM_CPU_H
