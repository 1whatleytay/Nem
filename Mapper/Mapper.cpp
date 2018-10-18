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

    Direction Mapper::getMirroring() {
        return rom->header.getMirroring();
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
            std::cout << getName() << ": Write " << (int)value << " to " << makeHex(address) << " PC: " << getDebugPC() << std::endl;
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

    class MMC1 : public Mapper {
        vector<Byte> ram = vector<Byte>(kilobyte(8));

        enum ControlFlags {
            Mirroring = 0b00000011,
            PRGMode   = 0b00001100,
            CHRMode   = 0b00010000,
        };

        enum PRGBankMode {
            Switch32KB,
            Switch16KBFixFirst,
            Switch16KBFixLast,
        };

        enum CHRBankMode {
            Switch8KB,
            Switch4KB,
        };

        int writeId = 0;
        Byte shift = 0;

        Byte control = 0b00010000;
        Byte chrBank[2] = { 0, 0 };
        Byte prgBank = 0;
    public:
        Direction getMirroring() override {
            switch (control & ControlFlags::Mirroring) {
                default:
                case 0:
                case 1:
                    //std::cout << getName() << ": Single screen mirroring is unsupported." << std::endl;
                case 2:
                    return Direction::Vertical;
                case 3:
                    return Direction::Horizontal;
            }
        }

        PRGBankMode getPRGBankMode() {
            switch ((control & ControlFlags::PRGMode) >> 2) {
                default:
                case 0:
                case 1:
                    return PRGBankMode::Switch32KB;
                case 2:
                    return PRGBankMode::Switch16KBFixFirst;
                case 3:
                    return PRGBankMode::Switch16KBFixLast;
            }
        }

        CHRBankMode getCHRBankMode() {
            switch ((control & ControlFlags::CHRMode) >> 4) {
                default:
                case 0:
                    return CHRBankMode::Switch8KB;
                case 1:
                    return CHRBankMode::Switch4KB;
            }
        }

        Byte getRAMByte(Address address) override {
            if ((prgBank & 0b00010000) == 0b00010000) {
                std::cout << getName() << ": RAM disabled. Refused read at " << makeHex(address) << std::endl;
                return 0;
            }
            return ram[address - CPUMemoryRegion::SRAM];
        }
        void setRAMByte(Address address, Byte value) override {
            if ((prgBank & 0b00010000) == 0b00010000) {
                std::cout << getName() << ": RAM disabled. Refused write at " << makeHex(address) << " <- " << (int)value << std::endl;
                return;
            }
            ram[address - CPUMemoryRegion::SRAM] = value;
        }

        Byte getPRGByte(Address address) override {
            int bank = getBankReference(address);
            switch (getPRGBankMode()) {
                case Switch32KB:
                    return rom->prgROM[address - CPUMemoryRegion::PRGRom +
                        ((prgBank & 0b00001110) >> 1) * kilobyte(32)];
                case Switch16KBFixFirst:
                    if (bank == 0) return rom->prgROM[address - CPUMemoryRegion::PRGRom];
                    else return rom->prgROM[address - CPUMemoryRegion::PRGRom +
                        (prgBank & 0b00001111) * kilobyte(16)];
                case Switch16KBFixLast:
                    if (bank == 0) return rom->prgROM[address - CPUMemoryRegion::PRGRom +
                        (prgBank & 0b00001111) * kilobyte(16)];
                    else return rom->prgROM[address - CPUMemoryRegion::PRGRom +
                        kilobyte(16)];
            }
        }
        void setPRGByte(Address address, Byte value) override {
            if ((value & 0b10000000) == 0b10000000) {
                shift = 0;
                writeId = 0;
            }

            shift = shift << 1;
            shift |= value & 0b00000001;
            writeId++;

            if (writeId >= 5) {
                if (address < 0xA000) {
                    control = shift;
                    ppuNeedsRefresh = true;
                } else if (address < 0xC000) {
                    chrBank[0] = shift;
                } else if (address < 0xE000) {
                    chrBank[1] = shift;
                } else {
                    prgBank = shift;
                }

                shift = 0;
                writeId = 0;
            }
        }

        Byte getCHRByte(Address address) override {
            if (rom->header.hasCHRRAM()) return chrRAM[address - PPUMemoryRegion::PatternTable0];
            switch (getCHRBankMode()) {
                case Switch8KB:
                    return rom->chrROM[address - PPUMemoryRegion::PatternTable0 +
                        ((chrBank[0] & 0b00011110) >> 1) * kilobyte(8)];
                case Switch4KB:
                    return rom->chrROM[address - PPUMemoryRegion::PatternTable0 +
                        (chrBank[address >= PPUMemoryRegion::PatternTable1] & 0b00011111) * kilobyte(4)];
            }
        }
        void setCHRByte(Address address, Byte value) override {
            if (rom->header.hasCHRRAM()) chrRAM[address - PPUMemoryRegion::PatternTable0] = value;
            else std::cout << getName() << ": Edit to unused CHR RAM." << std::endl;
        }

        explicit MMC1(ROM* nRom) : Mapper(1, nRom) { }
    };

    class CNROM : public Mapper {
        int bank = 0;
    public:
        Byte getRAMByte(Address address) override { return 0; }
        void setRAMByte(Address address, Byte value) override { }

        Byte getPRGByte(Address address) override {
            return rom->prgROM[address - CPUMemoryRegion::PRGRom];
        }
        void setPRGByte(Address address, Byte value) override {
            std::cout << "Switch Bank: " << (int)value << std::endl;
            bank = value & 0b00000011;
            ppuNeedsRefresh = true;
        }

        Byte getCHRByte(Address address) override {
            return rom->chrROM[(address - PPUMemoryRegion::PatternTable0) + bank * 0x2000];
        }
        void setCHRByte(Address address, Byte value) override {
            std::cout << getName() << ": Edit to unused CHR RAM." << std::endl;
        }

        explicit CNROM(ROM* nRom) : Mapper(3, nRom) { }
    };

    Mapper* UnimplementedMapper(ROM* rom) { return nullptr; }

    const std::function<Mapper*(ROM* rom)> mappers[16] = {
            [](ROM* rom) { return new NROM(rom); }, // Map 0x00
            [](ROM* rom) { return new MMC1(rom); }, // Map 0x01
            UnimplementedMapper, // Map 0x02
            [](ROM* rom) { return new CNROM(rom); }, // Map 0x03
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