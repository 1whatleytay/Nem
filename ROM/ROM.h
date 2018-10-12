//
// Created by Taylor Whatley on 2018-09-28.
//

#ifndef NEM_ROM_H
#define NEM_ROM_H

#include "../Internal.h"

namespace Nem {
    class ROMHeader {
    public:
        enum Mirroring {
            Vertical,
            Horizontal,
        };

        enum Flags {
            Flag6Mirroring     = 0b00000001,
            Flag6ExtraSaveRam  = 0b00000010,
            Flag6Trainer       = 0b00000100,
            Flag6FourScreen    = 0b00001000,
            Flag6LowMapper     = 0b11110000,

            Flag7VSUnisystem   = 0b00000001,
            Flag7Playchoice    = 0b00000010,

            Flag9IsPal         = 0b00000001,

            Flag10ExtraPRGRam  = 0b00010000,
            Flag10BusConflicts = 0b00100000,
        };

        Byte check[4], flag6, flag7, prgRomSize8K, flag9, flag10, zero[5];
        unsigned prgRomSize, chrRomSize;

        Byte getMapper() const;
        Mirroring getMirroring() const;
        bool hasTrainer() const;
        bool hasCHRRAM() const;
        bool zeroIsZero() const;
        bool verify() const;

        explicit ROMHeader(const vector<Byte>& romData);
    };

    class ROM {
        string romPath;

        const vector<Byte> romData;
    public:
        enum Version {
            Archaic,
            iNES,
            NES2,
        };

        const ROMHeader header;
        const vector<Byte> prgROM;
        const vector<Byte> chrROM;

        Version getVersion() const;

        string getRomName() const;
        string getRomInfo() const;

        explicit ROM(string pathToRom);
    };
}

#endif //NEM_ROM_H
