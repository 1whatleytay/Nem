//
// Created by Taylor Whatley on 2018-09-19.
//

#include "../Emulator.h"
#include "../Internal.h"
#include "../Errors.h"

#include "ROM.h"

#include <fstream>

namespace Nem {
    Byte headerVerify[4] = { 0x4E, 0x45, 0x53, 0x1A };

    ROMHeader::Mirroring ROMHeader::getMirroring() {
        return (flag6 & 0b00000001) == 0b00000001 ? Vertical : Horizontal;
    }
    bool ROMHeader::verify() { return memcmp(check, headerVerify, 4) == 0; }

    ROMHeader::ROMHeader(vector<Byte>& romData) {
        if (romData.size() < 16) throw RomInvalidException("too small.");
        memcpy(check, &romData[0], 4);
        prgRomSize = (unsigned)romData[4] * kilobyte(16);
        chrRomSize = (unsigned)romData[5] * kilobyte(8);

        flag6 = romData[6];

        // For iNES and NES 2.0
        flag7 = romData[7];
        prgRomSize8K = romData[8];
        flag9 = romData[9];
        flag10 = romData[10];

        memcpy(zero, &romData[11], 5);

        if (!verify()) throw RomInvalidException("not an NES rom.");
    }

    ROM::ROM(string& pathToRom) {
        std::ifstream stream(pathToRom, std::ios::binary | std::ios::in | std::ios::ate);
        if (!stream.good()) throw RomNotFoundException(pathToRom);
        vector<Byte> romData((unsigned long)stream.tellg());
        stream.seekg(0, std::ios::beg);
        stream.read((char*)&romData[0], romData.size());
        stream.close();

        header = ROMHeader(romData);
        prgROM = vector<Byte>(header.prgRomSize);
        chrROM = vector<Byte>(header.chrRomSize);
        memcpy(&prgROM[0], &romData[16], header.prgRomSize);
        memcpy(&chrROM[0], &romData[16 + header.prgRomSize], header.chrRomSize);
    }
}