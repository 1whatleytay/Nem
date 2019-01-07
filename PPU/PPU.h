//
// Created by Taylor Whatley on 2018-09-28.
//

#ifndef NEM_PPU_H
#define NEM_PPU_H

#include "../ROM/ROM.h"
#include "../Util/Ranges.h"

#include <mutex>

namespace Nem {
    class CPU;
    class Mapper;
    class Clock;

    int shiftAttribute(Byte var, int x, int y);

    struct PPUPalette { Byte colorA, colorB, colorC; };

    struct PPUPaletteMemory {
        Byte clearColor;
        PPUPalette background[4];
        PPUPalette sprite[4];
        Byte extra[3];
    };

    typedef Byte PPUNameTable[1024];
    typedef Byte PPUOAM[64 * 4];

    enum PPUMemoryRegion {
        PatternTables   = 0x0000,
        NameTables      = 0x2000,
        AttributeTables = 0x23C0,
        PaletteRam      = 0x3F00,
    };

    class PPUMemoryEdits {
    public:
        std::mutex mutex;

        Ranges patternTable[2]; // 0 - 255
        Ranges nameTable[4]; // 0 - 959
        Ranges oam; // 0 - 255

        bool paletteRam = false;
        bool registers = false;

        void fill();
        void reset();
    };

    class PPUMemory {
        Mapper* mapper;

        PPUNameTable nameTables[2];
        PPUOAM oam;
    public:
        PPUPaletteMemory palettes;
        PPUMemoryEdits edits;

        int getNameTableIndex(int index);
        int getNameTableByIndex(int nameTable);
        static Address regionIndex(PPUMemoryRegion region, int index);

        struct MappedAddress {
            PPUMemoryRegion region;
            Address effectiveAddress;
            int index;
        };

        MappedAddress mapAddress(Address address);

        Byte getByte(Address address);
        void setByte(Address address, Byte value);

        Byte getOAM(Byte address);
        void setOAM(Byte address, Byte value);

        void checkNeedsRefresh();

        explicit PPUMemory(Mapper* nMapper);
    };

    class PPURegisters {
    public:
        enum ControlFlags {
            NameTable       = 0b00000011,
            Increment       = 0b00000100,
            SprPatternTable = 0b00001000,
            BkgPatternTable = 0b00010000,
            SprSize         = 0b00100000,
            PPUMasterSlave  = 0b01000000,
            VBlankNMI       = 0b10000000,
        };

        enum MaskFlags {
            Grayscale   = 0b00000001,
            ShowBKGLeft = 0b00000010,
            ShowSPRLeft = 0b00000100,
            ShowBKG     = 0b00001000,
            ShowSPR     = 0b00010000,
            EmpRed      = 0b00100000,
            EmpGreen    = 0b01000000,
            EmpBlue     = 0b10000000,
        };

        enum StatusFlags {
            LowBits = 0b00011111,
            SprOverflow = 0b00100000,
            SprZeroHit  = 0b01000000,
            VBlank = 0b10000000,
        };

        bool flipSpriteZero = false;

        Byte control = 0b00000000;
        Byte mask    = 0b00000000;
        Byte status  = 0b10100000;

        Byte oamAddress = 0x00;

        bool scrollWrite = true;
        Byte scrollX = 0, scrollY = 0;

        bool addressWrite = true;
        Address address = 0x0000;
    };

    class PPU {
        CPU* cpu;

    public:
        PPUMemory memory;
        PPURegisters registers;

        bool oddFrame = false;

        long long ticks = 0;

        void postNMI();
        void clock();

        bool isControlSet(PPURegisters::ControlFlags flags);
        bool isMaskSet(PPURegisters::MaskFlags flags);

        Byte getOAM();
        void setOAM(Byte value);
        void sendOAMDMA(Byte page);

        void setCPU(Nem::CPU* nCPU);

        explicit PPU(Mapper* mapper);
    };

    struct Color { float red, green, blue; };

    // 2C02 Palette
    extern Color palette2C02[0x40];
}

#endif //NEM_PPU_H
