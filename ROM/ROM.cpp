//
// Created by Taylor Whatley on 2018-09-19.
//

#include "../Emulator.h"
#include "../Mapper/Mapper.h"
#include "../Errors.h"

#include "ROM.h"

#include <fstream>
#include <sstream>
#include <algorithm>

namespace Nem {
    Byte headerVerify[4] = { 0x4E, 0x45, 0x53, 0x1A };

    vector<Byte> loadRomData(string pathToRom) {
        std::ifstream stream(pathToRom, std::ios::binary | std::ios::in | std::ios::ate);
        if (!stream.good()) throw RomNotFoundException(pathToRom);
        vector<Byte> romData((unsigned long)stream.tellg());
        stream.seekg(0, std::ios::beg);
        stream.read((char*)&romData[0], romData.size());
        stream.close();

        return romData;
    }

    string getVersionName(ROM::Version version) {
        switch (version) {
            case ROM::Version::NES2: return "NES 2.0";
            case ROM::Version::iNES: return "iNES";
            case ROM::Version::Archaic: return "Archaic NES";
            default: return "Unknown";
        }
    }

    string getMirroringName(Direction mirroring) {
        switch (mirroring) {
            case Direction::Vertical: return "Vertical";
            case Direction::Horizontal: return "Horizontal";
            default: return "Unknown";
        }
    }

    Byte ROMHeader::getMapper() const {
        return (flag6 & Flag6LowMapper) >> 4;
    }

    Direction ROMHeader::getMirroring() const {
        return (flag6 & Flag6Mirroring) == Flag6Mirroring ? Vertical : Horizontal;
    }

    bool ROMHeader::hasTrainer() const {
        return (flag6 & Flag6Trainer) == Flag6Trainer;
    }

    bool ROMHeader::hasCHRRAM() const {
        return chrRomSize == 0;
    }

    bool ROMHeader::zeroIsZero() const {
        for (int a = 0; a < 5; a++) if (zero[a] != 0) return false;
        return true;
    }

    bool ROMHeader::verify() const {
        return memcmp(check, headerVerify, 4) == 0;
    }

    ROMHeader::ROMHeader(const vector<Byte>& romData) {
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

    ROM::Version ROM::getVersion() const {
        if (header.flag7 == 0x08 && header.zero[1] == 0x08 &&
            header.prgRomSize8K * kilobyte(8) <= prgROM.size()) return Version::NES2;
        if (header.flag7 != 0x00 && header.zeroIsZero()) return Version::iNES;
        return Version::Archaic;
    }

    string ROM::getRomName() const {
        unsigned long lastIndexFront = romPath.find_last_of('/'), lastIndexBack = romPath.find_last_of('\\');
        if (lastIndexFront == string::npos) lastIndexFront = 0;
        else lastIndexFront++;
        if (lastIndexBack == string::npos) lastIndexBack = 0;
        else lastIndexBack++;
        unsigned long lastIndex = std::max(lastIndexFront, lastIndexBack);
        return romPath.substr(lastIndex, romPath.length() - lastIndex);
    }

    string ROM::getRomInfo() const {
        std::stringstream stream;
        stream << getRomName() << " (" << getVersionName(getVersion()) << ") -"
        << " Mapper: (" << mapperNames[header.getMapper()] << " : " << (int)header.getMapper() << ")"
        << " Mirroring: (" << getMirroringName(header.getMirroring()) << ")"
        << " Trainer: (" << (header.hasTrainer() ? "true" : "false") << ")"
        << " CHR " << (header.hasCHRRAM() ? "RAM" : "ROM")
        << ": (" << (header.hasCHRRAM() ? "8192" : std::to_string(header.chrRomSize)) << ")";
        return stream.str();
    }

    ROM::ROM(string pathToRom) : romPath(pathToRom),
        romData(loadRomData(pathToRom)),
        header(ROMHeader(romData)),
        prgROM(&romData[0] + 16,
                &romData[0] + 16 + header.prgRomSize),
        chrROM(&romData[0] + 16 + header.prgRomSize,
                &romData[0] + 16 + header.prgRomSize + header.chrRomSize)
        { }
}