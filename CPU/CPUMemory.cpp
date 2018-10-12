//
// Created by Taylor Whatley on 2018-09-19.
//

#include "CPU.h"
#include "../PPU/PPU.h"
#include "../Mapper/Mapper.h"
#include "../Controller/Controller.h"
#include "../Errors.h"

#include <iostream>

namespace Nem {
    string regionName(CPUMemoryRegion region) {
        switch (region) {
            case CPUMemoryRegion::WorkRam: return "Work Ram";
            case CPUMemoryRegion::PPUIO: return "PPU Control Registers";
            case CPUMemoryRegion::APUIO: return "APU/IO Registers";
            case CPUMemoryRegion::Debug: return "Debug Memory";
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
            return { CPUMemoryRegion::Debug, address };
        else if (address < 0x8000)
            return { CPUMemoryRegion::SRAM, address };
        else
            return { CPUMemoryRegion::PRGRom, address };
    }

    Byte CPUMemory::getByte(Address address) {
        MappedAddress mappedAddress = mapAddress(address);
        switch (mappedAddress.region) {
            case WorkRam:
                return workRam[mappedAddress.effectiveAddress - mappedAddress.region];
            case PPUIO:
                switch (mappedAddress.effectiveAddress) {
                    case 0x2002:
                        ppu->registers->flipSpriteZero = !ppu->registers->flipSpriteZero;
                        return (Byte)(ppu->registers->status & ~0b01000000) | (Byte)( 0b01000000 * ppu->registers->flipSpriteZero);
                    case 0x2004:
                        return ppu->memory->oam[ppu->registers->oamAddress];
                    case 0x2007:
                        return ppu->memory->getByte(ppu->registers->address);
                    default: break;
                }
                break;
            case APUIO:
                switch (mappedAddress.effectiveAddress) {
                    case 0x4016:
                        if (controllers[0]) return controllers[0]->read();
                        std::cout << "Invalid controller0." << std::endl;
                        return 0;
                    case 0x4017:
                        if (controllers[1]) return controllers[1]->read();
                        std::cout << "Invalid controller1." << std::endl;
                        return 0;
                    default: break;
                }
                break;
            case Debug:
                break;
            case SRAM:
                return mapper->getRAMByte(mappedAddress.effectiveAddress);
            case PRGRom:
                return mapper->getPRGByte(mappedAddress.effectiveAddress);
                //return rom->prgROM[mappedAddress.effectiveAddress - mappedAddress.region];
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
                        ppu->memory->oam[ppu->registers->oamAddress] = value;
                        ppu->memory->edits.oam = true;
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
                switch (mappedAddress.effectiveAddress) {
                    case 0x4014:
                        for (Address a = 0; a < 0x100; a++) {
                            ppu->memory->oam[a] = getByte(value * (Address)0x100 + a);
                        }
                        ppu->memory->edits.oam = true;
                        cpu->waitCycles(553 + cpu->cycles % 2);
                        return;
                    case 0x4016:
                        if (controllers[0]) controllers[0]->write(value);
                        if (controllers[1]) controllers[1]->write(value);
                        return;
                    default: break;
                }
                break;
            case Debug:
                break;
            case SRAM:
                mapper->setRAMByte(mappedAddress.effectiveAddress, value);
                return;
            case PRGRom:
                mapper->setPRGByte(mappedAddress.effectiveAddress, value);
                return;
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
        return mapper->getNMIVector();
//        return makeAddress(rom->prgROM[0xFFFA - CPUMemoryRegion::PRGRom],
//                           rom->prgROM[0xFFFB - CPUMemoryRegion::PRGRom]);
    }
    Address CPUMemory::getResetVector() {
        return mapper->getResetVector();
//        return makeAddress(rom->prgROM[0xFFFC - CPUMemoryRegion::PRGRom],
//                           rom->prgROM[0xFFFD - CPUMemoryRegion::PRGRom]);
    }
    Address CPUMemory::getIRQVector() {
        return mapper->getIRQVector();
//        return makeAddress(rom->prgROM[0xFFFE - CPUMemoryRegion::PRGRom],
//                           rom->prgROM[0xFFFF - CPUMemoryRegion::PRGRom]);
    }

    void CPUMemory::setPPU(PPU* nPPU) { ppu = nPPU; }
    void CPUMemory::setController(int index, ControllerInterface* controller) { controllers[index] = controller; }

    CPUMemory::CPUMemory(CPU* nCPU, Mapper* nMapper) : cpu(nCPU), mapper(nMapper) {
//        setByte(0x4015, 0);
//        setByte(0x4017, 0);
//        for (Address a = 0x4000; a < 0x4010; a++) setByte(a, 0);
    }
}