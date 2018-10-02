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

        Byte check[4], flag6, flag7, prgRomSize8K, flag9, flag10, zero[5];
        unsigned prgRomSize, chrRomSize;

        Mirroring getMirroring();
        bool verify();

        ROMHeader() = default;
        explicit ROMHeader(vector<Byte>& romData);
    };

    class ROM {
    public:
        ROMHeader header;
        vector<Byte> prgROM;
        vector<Byte> chrROM;

        explicit ROM(string& pathToRom);
    };
}

#endif //NEM_ROM_H
