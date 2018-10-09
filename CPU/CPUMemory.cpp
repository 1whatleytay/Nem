//
// Created by Taylor Whatley on 2018-09-19.
//

#include "../Nem.h"
#include "../Errors.h"

#include <iostream>

namespace Nem {

    string regionName(CPUMemoryRegion region) {
        switch (region) {
            case CPUMemoryRegion::WorkRam: return "Work Ram";
            case CPUMemoryRegion::PPUIO: return "PPU Control Registers";
            case CPUMemoryRegion::APUIO: return "APU/IO Registers";
            case CPUMemoryRegion::SRAM: return "SRAM";
            case CPUMemoryRegion::PRGRom: return "Program ROM";
        }
    }

    CPUMemory::MappedAddress CPUMemory::mapAddress(Address address) {
        if (address < 0x2000)
            return { CPUMemoryRegion::WorkRam, (Address)(address % 0x0800) };
        else if (address < 0x4000)
            return { CPUMemoryRegion::PPUIO, (Address)((address - 0x2000) % 0x0008 + 0x2000) };
        else if (address < 0x4020)
            return { CPUMemoryRegion::APUIO, address };
        else if (address < 0x6000)
            return { CPUMemoryRegion::SRAM, address };
        else
            return { CPUMemoryRegion::PRGRom,
#ifdef MIRROR_ROM
                     (Address)((address - 0x8000) % 0x4000 + 0x8000)
#else
                     address
#endif
        };
    }

    Byte CPUMemory::getByte(Address address) {
        MappedAddress mappedAddress = mapAddress(address);
        switch (mappedAddress.region) {
            case WorkRam:
                return workRam[mappedAddress.effectiveAddress - mappedAddress.region];
            case PPUIO:
                switch (mappedAddress.effectiveAddress) {
                    case 0x2002:
                        return ppu->registers->status;
                    case 0x2004:
                        return ppu->registers->oamData;
                    case 0x2007:
                        return ppu->memory->getByte(ppu->registers->address);
                    default: break;
                }
                break;
            case APUIO:
                break;
            case SRAM:
                break;
            case PRGRom:
                return rom->prgROM[mappedAddress.effectiveAddress - mappedAddress.region];
        }
        std::cout << "Read @ $" << makeHex(address) << " is unimplemented!"
        << " Region: " << regionName(mappedAddress.region) << std::endl;
        return 0;
    }

    Address CPUMemory::getAddress(Address address) {

        return makeAddress(getByte(address), getByte(address + (Address)1));
    }

    void CPUMemory::setByte(Address address, Byte value) {
        MappedAddress mappedAddress = mapAddress(address);
        switch (mappedAddress.region) {
            case WorkRam:
                workRam[mappedAddress.effectiveAddress - mappedAddress.region] = value;
                return;
            case PPUIO:
                switch (mappedAddress.effectiveAddress) {
                    case 0x2000:
                        ppu->registers->control = value;
                        return;
                    case 0x2001:
                        ppu->registers->mask = value;
                        return;
                    case 0x2003:
                        ppu->registers->oamAddress = value;
                        return;
                    case 0x2004:
                        ppu->registers->oamData = value;
                        return;
                    case 0x2005:
                        ppu->registers->scroll = putByte(ppu->registers->scrollWrite, ppu->registers->scroll, value);
                        ppu->registers->scrollWrite = !ppu->registers->scrollWrite;
                        return;
                    case 0x2006:
                        ppu->registers->address = putByte(ppu->registers->addressWrite, ppu->registers->address, value);
                        ppu->registers->addressWrite = !ppu->registers->addressWrite;
                        return;
                    case 0x2007:
                        ppu->memory->setByte(ppu->registers->address, value);
                        ppu->registers->address += 1
                                + (ppu->isControlSet(PPURegisters::ControlFlags::Increment) ? 31 : 0);
                        return;
                    default: break;
                }
                break;
            case APUIO:
                break;
            case SRAM:
                break;
            case PRGRom:
                break;
        }
        std::cout << "Write @ $" << makeHex(address) << " is unimplemented!"
        << " Value: " << (int)value << " Region: " << regionName(mappedAddress.region) << std::endl;
    }

    void CPUMemory::setAddress(Address address, Address value) {
        Byte loByte = lo(value), hiByte = hi(value);
        setByte(address, loByte);
        setByte(address + (Address)1, hiByte);
    }

    void CPUMemory::list(Address start, Address count) {
        std::cout << "[" << std::endl;
        for (Address a = 0; a < count; a++) {
            Address loc = start + a;
            std::cout << "\t" << makeHex(loc) << ": " << makeHex(getByte(loc)) << std::endl;
        }
        std::cout << "]" << std::endl;
    }

    // Force disable program rom mirroring
    Address CPUMemory::getNMIVector() {
        return makeAddress(rom->prgROM[0xFFFA - CPUMemoryRegion::PRGRom],
                           rom->prgROM[0xFFFB - CPUMemoryRegion::PRGRom]);
    }
    Address CPUMemory::getResetVector() {
        return makeAddress(rom->prgROM[0xFFFC - CPUMemoryRegion::PRGRom],
                           rom->prgROM[0xFFFD - CPUMemoryRegion::PRGRom]);
    }
    Address CPUMemory::getIRQVector() {
        return makeAddress(rom->prgROM[0xFFFE - CPUMemoryRegion::PRGRom],
                           rom->prgROM[0xFFFF - CPUMemoryRegion::PRGRom]);
    }

    void CPUMemory::setPPU(PPU* nPPU) {
        ppu = nPPU;
    }

    CPUMemory::CPUMemory(Nem::ROM *nRom) {
        rom = nRom;

//        setByte(0x4015, 0);
//        setByte(0x4017, 0);
//        for (Address a = 0x4000; a < 0x4010; a++) setByte(a, 0);
    }
}