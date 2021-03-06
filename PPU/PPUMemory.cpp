//
// Created by Taylor Whatley on 2018-09-20.
//

#include "PPU.h"

#include "../Mapper/Mapper.h"

#include <iostream>

namespace Nem {
    string regionName(PPUMemoryRegion region) {
        switch (region) {
            case PatternTables: return "Pattern Tables";
            case NameTables: return "Name Tables";
            case AttributeTables: return "Attribute Tables";
            case PaletteRam: return "Palette Ram";
            default: return "Unknown";
        }
    }

    PPUMemory::MappedAddress PPUMemory::mapAddress(Address address) {
        if (address < 0x2000)
            return { PatternTables, address, address / (Address)0x1000 };
        else if (address < 0x3000)
            return { ((address - 0x2000) % 0x400 >= 0x3C0) ? AttributeTables : NameTables,
                     address, (address - 0x2000) / 0x400};
        else if (address < 0x3F00)
            return { ((address - 0x3000) % 0x400 >= 0x3C0) ? AttributeTables : NameTables,
                     address, (address - 0x3000) / 0x400};
        else
            return { PPUMemoryRegion::PaletteRam, (Address)((address - 0x3F00) % 0x0020 + 0x3F00) };
    }

    int PPUMemory::getNameTableIndex(int index) {
        switch (mapper->getMirroring()) {
            case Direction::Vertical:
                switch (index) {
                    case 0: return 0;
                    case 1: return 1;
                    case 2: return 0;
                    case 3: return 1;
                    default: return 0;
                }
            case Direction::Horizontal:
                switch (index) {
                    case 0: return 0;
                    case 1: return 0;
                    case 2: return 1;
                    case 3: return 1;
                    default: return 0;
                }
        }
        return 0;
    }

    Address PPUMemory::regionIndex(Nem::PPUMemoryRegion region, int index) {
        switch (region) {
            case PatternTables:
                return PatternTables + 0x1000 * index;
            case NameTables:
                return NameTables + 0x400 * index;
            case AttributeTables:
                return AttributeTables + 0x400 * index;
            default:
                return region;
        }
    }

    int PPUMemory::getNameTableByIndex(int nameTable) {
        switch (mapper->getMirroring()) {
            case Direction::Vertical:
                switch (nameTable) {
                    case 0: return 0;
                    case 1: return 1;
                    default: return 0;
                }
            case Direction::Horizontal:
                switch (nameTable) {
                    case 0: return 0;
                    case 1: return 1;
                    default: return 0;
                }
        }
        return 0;
    }

    Byte PPUMemory::getByte(Address address) {
        MappedAddress mappedAddress = mapAddress(address);
        switch (mappedAddress.region) {
            case PatternTables:
                return mapper->getCHRByte(mappedAddress.effectiveAddress);
            case AttributeTables:
            case NameTables:
                return nameTables[getNameTableByIndex(mappedAddress.index)]
                    [mappedAddress.effectiveAddress - regionIndex(NameTables, mappedAddress.index)];
            case PaletteRam:
                switch (mappedAddress.effectiveAddress) {
                    case 0x3F10:
                    case 0x3F00: return palettes.clearColor;

                    case 0x3F14:
                    case 0x3F04: return palettes.extra[0];

                    case 0x3F18:
                    case 0x3F08: return palettes.extra[1];

                    case 0x3F1C:
                    case 0x3F0C: return palettes.extra[2];

                    case 0x3F01: return palettes.background[0].colorA;
                    case 0x3F02: return palettes.background[0].colorB;
                    case 0x3F03: return palettes.background[0].colorC;

                    case 0x3F05: return palettes.background[1].colorA;
                    case 0x3F06: return palettes.background[1].colorB;
                    case 0x3F07: return palettes.background[1].colorC;

                    case 0x3F09: return palettes.background[2].colorA;
                    case 0x3F0A: return palettes.background[2].colorB;
                    case 0x3F0B: return palettes.background[2].colorC;

                    case 0x3F0D: return palettes.background[3].colorA;
                    case 0x3F0E: return palettes.background[3].colorB;
                    case 0x3F0F: return palettes.background[3].colorC;

                    case 0x3F11: return palettes.sprite[0].colorA;
                    case 0x3F12: return palettes.sprite[0].colorB;
                    case 0x3F13: return palettes.sprite[0].colorC;

                    case 0x3F15: return palettes.sprite[1].colorA;
                    case 0x3F16: return palettes.sprite[1].colorB;
                    case 0x3F17: return palettes.sprite[1].colorC;

                    case 0x3F19: return palettes.sprite[2].colorA;
                    case 0x3F1A: return palettes.sprite[2].colorB;
                    case 0x3F1B: return palettes.sprite[2].colorC;

                    case 0x3F1D: return palettes.sprite[3].colorA;
                    case 0x3F1E: return palettes.sprite[3].colorB;
                    case 0x3F1F: return palettes.sprite[3].colorC;

                    default: break;
                }
                break;
        }
        std::cout << "PPU Read @ $" << makeHex(address) << " is unimplemented!"
                  << " Region: " << regionName(mappedAddress.region) << std::endl;
        return 0;
    }

    void PPUMemory::setByte(Address address, Byte value) {
        MappedAddress mappedAddress = mapAddress(address);
        edits.mutex.lock();
        bool set = false;
        int arrIndex, point, x, y;
        switch (mappedAddress.region) {
            case PatternTables:
                mapper->setCHRByte(mappedAddress.effectiveAddress, value);
                edits.patternTable[mappedAddress.index].add(
                        (mappedAddress.effectiveAddress - regionIndex(
                                PatternTables, mappedAddress.index)) / (Address)16);
                set = true; break;
            case NameTables:
                arrIndex = mappedAddress.effectiveAddress - regionIndex(NameTables, mappedAddress.index);
                nameTables[getNameTableIndex(mappedAddress.index)][arrIndex] = value;
                edits.nameTable[mappedAddress.index].add(arrIndex);
                set = true; break;
            case AttributeTables:
                point = mappedAddress.effectiveAddress - regionIndex(NameTables, mappedAddress.index);
                arrIndex = mappedAddress.effectiveAddress - regionIndex(AttributeTables, mappedAddress.index);
                x = arrIndex % 8; y = arrIndex / 8;
                nameTables[getNameTableIndex(mappedAddress.index)][point] = value;
                for (int a = 0; a < 4; a++) {
                    for (int b = 0; b < 4 && y * 4 + b < 30; b++) {
                        edits.nameTable[mappedAddress.index].add(x * 4 + a + (y * 4 + b) * 32);
                    }
                }
                set = true; break;
            case PaletteRam:
                edits.paletteRam = true;
                switch (mappedAddress.effectiveAddress) {
                    case 0x3F10:
                    case 0x3F00: palettes.clearColor = value; set = true; break;

                    case 0x3F14:
                    case 0x3F04: palettes.extra[0] = value; set = true; break;

                    case 0x3F18:
                    case 0x3F08: palettes.extra[1] = value; set = true; break;

                    case 0x3F1C:
                    case 0x3F0C: palettes.extra[2] = value; set = true; break;

                    case 0x3F01: palettes.background[0].colorA = value; set = true; break;
                    case 0x3F02: palettes.background[0].colorB = value; set = true; break;
                    case 0x3F03: palettes.background[0].colorC = value; set = true; break;

                    case 0x3F05: palettes.background[1].colorA = value; set = true; break;
                    case 0x3F06: palettes.background[1].colorB = value; set = true; break;
                    case 0x3F07: palettes.background[1].colorC = value; set = true; break;

                    case 0x3F09: palettes.background[2].colorA = value; set = true; break;
                    case 0x3F0A: palettes.background[2].colorB = value; set = true; break;
                    case 0x3F0B: palettes.background[2].colorC = value; set = true; break;

                    case 0x3F0D: palettes.background[3].colorA = value; set = true; break;
                    case 0x3F0E: palettes.background[3].colorB = value; set = true; break;
                    case 0x3F0F: palettes.background[3].colorC = value; set = true; break;

                    case 0x3F11: palettes.sprite[0].colorA = value; set = true; break;
                    case 0x3F12: palettes.sprite[0].colorB = value; set = true; break;
                    case 0x3F13: palettes.sprite[0].colorC = value; set = true; break;

                    case 0x3F15: palettes.sprite[1].colorA = value; set = true; break;
                    case 0x3F16: palettes.sprite[1].colorB = value; set = true; break;
                    case 0x3F17: palettes.sprite[1].colorC = value; set = true; break;

                    case 0x3F19: palettes.sprite[2].colorA = value; set = true; break;
                    case 0x3F1A: palettes.sprite[2].colorB = value; set = true; break;
                    case 0x3F1B: palettes.sprite[2].colorC = value; set = true; break;

                    case 0x3F1D: palettes.sprite[3].colorA = value; set = true; break;
                    case 0x3F1E: palettes.sprite[3].colorB = value; set = true; break;
                    case 0x3F1F: palettes.sprite[3].colorC = value; set = true; break;

                    default: break;
                }
                break;
            default: break;
        }
        edits.mutex.unlock();
        if (set) return;

        std::cout << "PPU Write @ $" << makeHex(address) << " is unimplemented!"
                  << " Value: " << (int)value << " Region: "
                  << regionName(mappedAddress.region) << std::endl;
    }

    Byte PPUMemory::getOAM(Byte address) {
        return oam[address];
    }
    void PPUMemory::setOAM(Byte address, Byte value) {
        oam[address] = value;
        edits.mutex.lock();
        edits.oam.add(address / 4);
        edits.mutex.unlock();
    }

    void PPUMemory::checkNeedsRefresh() {
        if (mapper->ppuNeedsRefresh) {
            mapper->ppuNeedsRefresh = false;
            for (Ranges& range : edits.patternTable) range.fill(0, 960);
        }
    }

    PPUMemory::PPUMemory(Mapper* nMapper) : mapper(nMapper) { }

    void PPUMemoryEdits::fill() {
        for (Ranges& table : patternTable) table.fill(0, 256);
        for (Ranges& table : nameTable) table.fill(0, 0x3C0);
        oam.fill(0, 64);

        paletteRam = true;
        registers = true;
    }

    void PPUMemoryEdits::reset() {
        for (Ranges& table : patternTable) table.ranges.clear();
        for (Ranges& table : nameTable) table.ranges.clear();
        oam.ranges.clear();

        paletteRam = false;
        registers = false;
    }
}