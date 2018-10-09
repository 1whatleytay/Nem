//
// Created by Taylor Whatley on 2018-09-28.
//

#ifndef NEM_PPU_H
#define NEM_PPU_H

#include "../Internal.h"
#include "../ROM/ROM.h"

namespace Nem {
    class CPU;
    class Clock;

    struct PPUPalette { Byte colorA, colorB, colorC; };

    struct PPUPaletteMemory {
        Byte clearColor;
        PPUPalette background[4];
        PPUPalette sprite[4];
    };

    typedef Byte PPUNameTable[1024];

    enum PPUMemoryRegion {
        PatternTable0 = 0x0000,
        PatternTable1 = 0x1000,
        NameTable0    = 0x2000,
        NameTable1    = 0x2400,
        NameTable2    = 0x2800,
        NameTable3    = 0x2C00,
        PaletteRam    = 0x3F00,
    };

    class PPUMemoryEdits {
    public:
        bool patternTable = false;
        bool nameTable = false;
        bool paletteRam = false;

        void addEdit(PPUMemoryRegion region);
        void reset();
    };

    class PPUMemory {
        ROMHeader::Mirroring mirroring;

        // 0: top left, 1: top right, 2: bottom left, 3: bottom right
        PPUNameTable nameTables[2];

        ROM* rom = nullptr;
    public:
        PPUMemoryEdits edits;

        int getNameTableIndex(int index);
        int getNameTableByIndex(int nameTable);

        PPUPaletteMemory palettes;

        struct MappedAddress {
            PPUMemoryRegion region;
            Address effectiveAddress;
        };

        MappedAddress mapAddress(Address address);

        Byte getByte(Address address);
        Address getAddress(Address address);
        void setByte(Address address, Byte value);
        void setAddress(Address address, Address value);

        explicit PPUMemory(ROM* nRom);
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

        Byte control = 0b00000000;
        Byte mask    = 0b00000000;
        Byte status  = 0b10100000;

        Byte oamAddress = 0x00, oamData = 0x00;

        bool scrollWrite = true;
        Address scroll = 0x0000;

        bool addressWrite = true;
        Address address = 0x0000;
    };

    class PPU {
        Clock* masterClock;

        CPU* cpu;

    public:
        PPUMemory* memory;
        PPURegisters* registers;

        bool oddFrame = false;

        void nextFrame();
        void waitUntilNextFrame();

        bool isControlSet(PPURegisters::ControlFlags flags);
        bool isMaskSet(PPURegisters::MaskFlags flags);

        void setCPU(Nem::CPU* nCPU);

        PPU(Clock* nMasterClock, ROM* nROM);
        ~PPU();
    };

    struct Color { float red, green, blue; };

    // 2C02 Palette
    extern Color palette2C02[];
}

#endif //NEM_PPU_H
