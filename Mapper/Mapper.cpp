//
// Created by Taylor Whatley on 2018-10-11.
//

#include "Mapper.h"

#include "../CPU/CPU.h"
#include "../PPU/PPU.h"
#include "../ROM/ROM.h"

#include <iostream>

namespace Nem {
    int Mapper::getBankReference(Address address) {
        return address >= 0xC000;
    }

    string Mapper::getName() { return mapperNames[index]; }

    const ROMHeader* Mapper::getHeader() {
        return &rom->header;
    }

    Address Mapper::getNMIVector() {
        return makeAddress(getPRGByte(0xFFFA), getPRGByte(0xFFFB));
    }
    Address Mapper::getResetVector() {
        return makeAddress(getPRGByte(0xFFFC), getPRGByte(0xFFFD));
    }
    Address Mapper::getIRQVector() {
        return makeAddress(getPRGByte(0xFFFE), getPRGByte(0xFFFF));
    }

    Mapper::Mapper(int nIndex, ROM* nRom) : index(nIndex), rom(nRom), chrRAM(nRom->header.hasCHRRAM() * 0x2000ul) { }

    class NROM : public Mapper {
        vector<Byte> ram = vector<Byte>(kilobyte(4));
    public:
        Byte getRAMByte(Address address) override {
            return ram[address - CPUMemoryRegion::SRAM];
        }
        void setRAMByte(Address address, Byte value) override {
            ram[address - CPUMemoryRegion::SRAM] = value;
        }

        Byte getPRGByte(Address address) override {
            int mirror = 0;
            if (rom->header.prgRomSize <= kilobyte(16)) mirror = getBankReference(address) * 0x4000;
            return rom->prgROM[address - CPUMemoryRegion::PRGRom - mirror];
        }
        void setPRGByte(Address address, Byte value) override {
            std::cout << getName() << ": Attempt to switch banks unsupported." << std::endl;
        }

        Byte getCHRByte(Address address) override {
            if (rom->header.hasCHRRAM()) return chrRAM[address - PPUMemoryRegion::PatternTable0];
            else return rom->chrROM[address - PPUMemoryRegion::PatternTable0];
        }
        void setCHRByte(Address address, Byte value) override {
            if (rom->header.hasCHRRAM()) chrRAM[address - PPUMemoryRegion::PatternTable0] = value;
            else std::cout << getName() << ": Edit to unused CHR RAM." << std::endl;
        }

        explicit NROM(ROM* nRom) : Mapper(0, nRom) { }
    };

    Mapper* UnimplementedMapper(ROM* rom) { return nullptr; }

    const std::function<Mapper*(ROM* rom)> mappers[16] = {
            [](ROM* rom) { return new NROM(rom); }, // Map 0x00
            UnimplementedMapper, // Map 0x01
            UnimplementedMapper, // Map 0x02
            UnimplementedMapper, // Map 0x03
            UnimplementedMapper, // Map 0x04
            UnimplementedMapper, // Map 0x05
            UnimplementedMapper, // Map 0x06
            UnimplementedMapper, // Map 0x07
            UnimplementedMapper, // Map 0x08
            UnimplementedMapper, // Map 0x09
            UnimplementedMapper, // Map 0x0A
            UnimplementedMapper, // Map 0x0B
            UnimplementedMapper, // Map 0x0C
            UnimplementedMapper, // Map 0x0D
            UnimplementedMapper, // Map 0x0E
            UnimplementedMapper, // Map 0x0F
    };

    const string mapperNames[16] = {
            "NROM",
            "MMC1",
            "UxROM",
            "CNROM",
            "Uni",
            "Uni",
            "Uni",
            "Uni",
            "Uni",
            "Uni",
            "Uni",
            "Uni",
            "Uni",
            "Uni",
            "Uni",
            "Uni",
    };
}