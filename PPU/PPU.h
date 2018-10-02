//
// Created by Taylor Whatley on 2018-09-28.
//

#ifndef NEM_PPU_H
#define NEM_PPU_H

#include "../Internal.h"

#include "../Timer.h"

namespace Nem {
    class ROM;
    class CPU;

    struct PPUPalette { Byte colorA, colorB, colorC; };

    class PPUPaletteMemory {
        Byte clearColor;
        PPUPalette background[4];
        PPUPalette sprite[4];

    public:
        std::function<void(Byte value)> clearColorListener;
        std::function<void(int index, PPUPalette palette)> backgroundListener;
        std::function<void(int index, PPUPalette palette)> spriteListener;

        void setClearColor(Byte value);
        void setBackgroundPalette(int index, PPUPalette palette);
        void setSpritePalette(int index, PPUPalette palette);

        Byte getClearColor();
        PPUPalette getBackgroundPalette(int index);
        PPUPalette getSpritePalette(int index);
    };

    struct PPUPatternTable {
        Byte data[16 * 16 * 16];
    };

    struct PPUNameTable {
        Byte data[32 * 30];
        Byte attributeTable[64];
    };

    class PPUMemory {
        PPUNameTable realNameTable0, realNameTable1;

        // 0: top left, 1: top right, 2: bottom left, 3: bottom right
        PPUNameTable *nameTable0 = nullptr, *nameTable1 = nullptr, *nameTable2 = nullptr, *nameTable3 = nullptr;

        ROM* rom = nullptr;
    public:
        PPUPaletteMemory palettes;

        enum Region {
            PatternTable0 = 0x0000,
            PatternTable1 = 0x1000,
            NameTable0    = 0x2000,
            NameTable1    = 0x2400,
            NameTable2    = 0x2800,
            NameTable3    = 0x2C00,
            PaletteRam    = 0x3F00,
        };

        struct MappedAddress {
            Region region;
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

        bool scrollWrite = false;
        Address scroll = 0x0000;

        bool addressWrite = false;
        Address address = 0x0000;
    };

    class PPU {
        CPU* cpu;

    public:
        PPUMemory* memory;
        PPURegisters* registers;

        void vblank();

        void setCPU(Nem::CPU* nCPU);

        explicit PPU(ROM* nROM);
        ~PPU();
    };

    struct Color { float red, green, blue; };

    // 2C02 Palette
    extern Color palette2C02[];
}

#endif //NEM_PPU_H
