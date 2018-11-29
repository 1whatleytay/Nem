//
// Created by Taylor Whatley on 2018-09-19.
//

#include "CPU.h"
#include "../PPU/PPU.h"
#include "../Mapper/Mapper.h"
#include "../Controller/Controller.h"
#include "../Errors.h"
#include "../APU/APU.h"

#ifdef NEM_PROFILE
#include "../Debug/Profiler.h"
#endif

#include <iostream>

namespace Nem {
    string regionName(CPUMemoryRegion region) {
        switch (region) {
            case CPUMemoryRegion::WorkRam: return "Work Ram";
            case CPUMemoryRegion::PPUIO: return "PPU Control Registers";
            case CPUMemoryRegion::GeneralIO: return "APU/IO Registers";
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
            return { CPUMemoryRegion::GeneralIO, address };
        else if (address < 0x6000)
            return { CPUMemoryRegion::Debug, address };
        else if (address < 0x8000)
            return { CPUMemoryRegion::SRAM, address };
        else
            return { CPUMemoryRegion::PRGRom, address };
    }

    Byte CPUMemory::getByte(Address address, bool cycle) {
        MappedAddress mappedAddress = mapAddress(address);
        if (cycle) cpu->readCycle();
        switch (mappedAddress.region) {
            case WorkRam:
                return workRam[mappedAddress.effectiveAddress - mappedAddress.region];
            case PPUIO:
                switch (mappedAddress.effectiveAddress) {
                    case 0x2002:
#ifdef FORCE_SPRITE_ZERO_HIT
                        ppu->registers->flipSpriteZero = !ppu->registers->flipSpriteZero;
                        return (Byte)(ppu->registers->status & ~0b01000000) |
                            (Byte)(0b01000000 * ppu->registers->flipSpriteZero);
#else
                        return ppu->registers->status;
#endif
                    case 0x2004:
                        return ppu->memory->oam[ppu->registers->oamAddress];
                    case 0x2007:
                        return ppu->memory->getByte(ppu->registers->address);
                    default: break;
                }
                break;
            case GeneralIO:
                switch (mappedAddress.effectiveAddress) {
                    case 0x4000: return apu->registers->pulse1.envelope;
                    case 0x4001: return apu->registers->pulse1.sweep;
                    case 0x4002: return apu->registers->pulse1.timer;
                    case 0x4003: return apu->registers->pulse1.length;

                    case 0x4004: return apu->registers->pulse2.envelope;
                    case 0x4005: return apu->registers->pulse2.sweep;
                    case 0x4006: return apu->registers->pulse2.timer;
                    case 0x4007: return apu->registers->pulse2.length;

                    case 0x4008: return apu->registers->triangle.control;
                    case 0x4009: return apu->registers->triangle.extra;
                    case 0x400A: return apu->registers->triangle.timer;
                    case 0x400B: return apu->registers->triangle.length;

                    case 0x400C: return apu->registers->noise.envelope;
                    case 0x400D: return apu->registers->noise.extra;
                    case 0x400E: return apu->registers->noise.loop;
                    case 0x400F: return apu->registers->noise.length;

                    case 0x4010: return apu->registers->dmc.loop;
                    case 0x4011: return apu->registers->dmc.load;
                    case 0x4012: return apu->registers->dmc.address;
                    case 0x4013: return apu->registers->dmc.length;

                    case 0x4015: return apu->registers->channels;

                    case 0x4016:
                        if (controllers[0]) return controllers[0]->read();
                        return 0;
                    case 0x4017:
                        if (controllers[1]) return controllers[1]->read();
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
        }
        std::cout << "Read @ $" << makeHex(address) << " is unimplemented!"
        << " Region: " << regionName(mappedAddress.region) << std::endl;

#ifdef NEM_PROFILE
        cpu->profiler->breakpoint();
#endif

        return 0;
    }

    Address CPUMemory::getAddress(Address address, bool cycle) {
        return makeAddress(getByte(address, cycle), getByte(address + (Address)1, cycle));
    }

    void CPUMemory::setByte(Address address, Byte value, bool cycle) {
        MappedAddress mappedAddress = mapAddress(address);
        if (cycle) cpu->writeCycle();
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
                        ppu->memory->edits.registers = true;
                        return;
                    case 0x2003:
                        ppu->registers->oamAddress = value;
                        return;
                    case 0x2004:
                        ppu->memory->oam[ppu->registers->oamAddress] = value;
                        ppu->memory->edits.oam = true;
                        return;
                    case 0x2005:
                        if (value > 0) {
                            if (ppu->registers->scrollWrite) ppu->registers->scrollX = value;
                            else ppu->registers->scrollY = value;
                        }
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
            case GeneralIO:
                switch (mappedAddress.effectiveAddress) {
                    case 0x4000: apu->registers->pulse1.envelope = value; return;
                    case 0x4001: apu->registers->pulse1.sweep = value; return;
                    case 0x4002: apu->registers->pulse1.timer = value; return;
                    case 0x4003: apu->registers->pulse1.length = value; return;

                    case 0x4004: apu->registers->pulse2.envelope = value; return;
                    case 0x4005: apu->registers->pulse2.sweep = value; return;
                    case 0x4006: apu->registers->pulse2.timer = value; return;
                    case 0x4007: apu->registers->pulse2.length = value; return;

                    case 0x4008: apu->registers->triangle.control = value; return;
                    case 0x4009: apu->registers->triangle.extra = value; return;
                    case 0x400A: apu->registers->triangle.timer = value; return;
                    case 0x400B: apu->registers->triangle.length = value; return;

                    case 0x400C: apu->registers->noise.envelope = value; return;
                    case 0x400D: apu->registers->noise.extra = value; return;
                    case 0x400E: apu->registers->noise.loop = value; return;
                    case 0x400F: apu->registers->noise.length = value; return;

                    case 0x4010: apu->registers->dmc.loop = value; return;
                    case 0x4011: apu->registers->dmc.load = value; return;
                    case 0x4012: apu->registers->dmc.address = value; return;
                    case 0x4013: apu->registers->dmc.length = value; return;

                    case 0x4015: apu->registers->channels = value; return;
                    case 0x4017: apu->registers->frameCounter = value; return;

                    case 0x4014:
                        for (Address a = 0; a < 0x100; a++) {
                            ppu->memory->oam[a] = getByte(value * (Address)0x100 + a);
                            cpu->writeCycle();
                        }
                        if (cpu->cycles % 2 == 0) cpu->readCycle();
                        ppu->memory->edits.oam = true;
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

#ifdef NEM_PROFILE
        cpu->profiler->breakpoint();
#endif
    }

    void CPUMemory::setAddress(Address address, Address value, bool cycle) {
        Byte loByte = lo(value), hiByte = hi(value);
        setByte(address, loByte, cycle);
        setByte(address + (Address)1, hiByte, cycle);
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
    }
    Address CPUMemory::getResetVector() {
        return mapper->getResetVector();
    }
    Address CPUMemory::getIRQVector() {
        return mapper->getIRQVector();
    }

    void CPUMemory::setPPU(PPU* nPPU) { ppu = nPPU; }
    void CPUMemory::setAPU(APU *nAPU) { apu = nAPU; }
    void CPUMemory::setController(int index, ControllerInterface* controller) { controllers[index] = controller; }

    CPUMemory::CPUMemory(CPU* nCPU, Mapper* nMapper) : cpu(nCPU), mapper(nMapper) { }
}