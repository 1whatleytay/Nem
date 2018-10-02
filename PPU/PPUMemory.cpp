//
// Created by Taylor Whatley on 2018-09-20.
//

#include "PPU.h"

#include "../ROM/ROM.h"

#include <iostream>

namespace Nem {
    string regionName(PPUMemory::Region region) {
        switch (region) {
            case PPUMemory::Region::PatternTable0: return "Pattern Table 0 (LEFT)";
            case PPUMemory::Region::PatternTable1: return "Pattern Table 1 (RIGHT)";
            case PPUMemory::Region::NameTable0: return "Name Table 0";
            case PPUMemory::Region::NameTable1: return "Name Table 1";
            case PPUMemory::Region::NameTable2: return "Name Table 2";
            case PPUMemory::Region::NameTable3: return "Name Table 3";
            case PPUMemory::Region::PaletteRam: return "Palette Ram";
        }
    }

    PPUMemory::MappedAddress PPUMemory::mapAddress(Address address) {
        if (address < 0x1000)
            return { Region::PatternTable0, address };
        else if (address < 0x2000)
            return { Region::PatternTable1, address };
        else if (address < 0x2400)
            return { Region::NameTable0, address };
        else if (address < 0x2800)
            return { Region::NameTable1, address };
        else if (address < 0x2C00)
            return { Region::NameTable2, address };
        else if (address < 0x3000)
            return { Region::NameTable3, address };
        else if (address < 0x3400)
            return { Region::NameTable0, (Address)(address - 0x1000) };
        else if (address < 0x3800)
            return { Region::NameTable1, (Address)(address - 0x1000) };
        else if (address < 0x3C00)
            return { Region::NameTable2, (Address)(address - 0x1000) };
        else if (address < 0x3F00)
            return { Region::NameTable3, (Address)(address - 0x1000) };
        else
            return { Region::PaletteRam, (Address)((address - 0x3F00) % 0x0020 + 0x3F00) };
    }

    Byte PPUMemory::getByte(Address address) {
        MappedAddress mappedAddress = mapAddress(address);
        switch (mappedAddress.region) {
            case PatternTable0:
                //return ((Byte*)&patternTable0)[mappedAddress.effectiveAddress - mappedAddress.region];
                return rom->chrROM[mappedAddress.effectiveAddress - mappedAddress.region];
            case PatternTable1:
                //return ((Byte*)&patternTable1)[mappedAddress.effectiveAddress - mappedAddress.region];
                return rom->chrROM[mappedAddress.effectiveAddress - mappedAddress.region + PatternTable0];
            case NameTable0:
                return ((Byte*)nameTable0)[mappedAddress.effectiveAddress - mappedAddress.region];
            case NameTable1:
                return ((Byte*)nameTable1)[mappedAddress.effectiveAddress - mappedAddress.region];
            case NameTable2:
                return ((Byte*)nameTable2)[mappedAddress.effectiveAddress - mappedAddress.region];
            case NameTable3:
                return ((Byte*)nameTable3)[mappedAddress.effectiveAddress - mappedAddress.region];
            case PaletteRam:
                switch (mappedAddress.effectiveAddress) {
                    case 0x3F10:
                    case 0x3F00: return palettes.getClearColor();

                    case 0x3F01: return palettes.getBackgroundPalette(0).colorA;
                    case 0x3F02: return palettes.getBackgroundPalette(0).colorB;
                    case 0x3F03: return palettes.getBackgroundPalette(0).colorC;

                    case 0x3F05: return palettes.getBackgroundPalette(1).colorA;
                    case 0x3F06: return palettes.getBackgroundPalette(1).colorB;
                    case 0x3F07: return palettes.getBackgroundPalette(1).colorC;

                    case 0x3F09: return palettes.getBackgroundPalette(2).colorA;
                    case 0x3F0A: return palettes.getBackgroundPalette(2).colorB;
                    case 0x3F0B: return palettes.getBackgroundPalette(2).colorC;

                    case 0x3F0D: return palettes.getBackgroundPalette(3).colorA;
                    case 0x3F0E: return palettes.getBackgroundPalette(3).colorB;
                    case 0x3F0F: return palettes.getBackgroundPalette(3).colorC;

                    case 0x3F11: return palettes.getSpritePalette(0).colorA;
                    case 0x3F12: return palettes.getSpritePalette(0).colorB;
                    case 0x3F13: return palettes.getSpritePalette(0).colorC;

                    case 0x3F15: return palettes.getSpritePalette(1).colorA;
                    case 0x3F16: return palettes.getSpritePalette(1).colorB;
                    case 0x3F17: return palettes.getSpritePalette(1).colorC;

                    case 0x3F19: return palettes.getSpritePalette(2).colorA;
                    case 0x3F1A: return palettes.getSpritePalette(2).colorB;
                    case 0x3F1B: return palettes.getSpritePalette(2).colorC;

                    case 0x3F1D: return palettes.getSpritePalette(3).colorA;
                    case 0x3F1E: return palettes.getSpritePalette(3).colorB;
                    case 0x3F1F: return palettes.getSpritePalette(3).colorC;

                    default: break;
                }
                break;
        }
        std::cout << "PPU Read @ $" << makeHex(address) << " is unimplemented!"
                  << " Region: " << regionName(mappedAddress.region) << std::endl;
        return 0;
    }
    Address PPUMemory::getAddress(Address address) {
        return makeAddress(getByte(address), getByte(address + (Address)1));
    }
    void PPUMemory::setByte(Address address, Byte value) {
        MappedAddress mappedAddress = mapAddress(address);
        PPUPalette palette;
        switch (mappedAddress.region) {
            case PatternTable0:
                //((Byte*)&patternTable0)[mappedAddress.effectiveAddress - mappedAddress.region] = value;
            case PatternTable1:
                //((Byte*)&patternTable1)[mappedAddress.effectiveAddress - mappedAddress.region] = value;
                break;
            case NameTable0:
                ((Byte*)nameTable0)[mappedAddress.effectiveAddress - mappedAddress.region] = value;
            case NameTable1:
                ((Byte*)nameTable1)[mappedAddress.effectiveAddress - mappedAddress.region] = value;
            case NameTable2:
                ((Byte*)nameTable2)[mappedAddress.effectiveAddress - mappedAddress.region] = value;
            case NameTable3:
                ((Byte*)nameTable3)[mappedAddress.effectiveAddress - mappedAddress.region] = value;
            case PaletteRam:
                switch (mappedAddress.effectiveAddress) {
                    case 0x3F10:
                    case 0x3F00: palettes.setClearColor(value);

                    case 0x3F01:
                        palette = palettes.getBackgroundPalette(0);
                        palette.colorA = value;
                        palettes.setBackgroundPalette(0, palette);
                    case 0x3F02:
                        palette = palettes.getBackgroundPalette(0);
                        palette.colorB = value;
                        palettes.setBackgroundPalette(0, palette);
                    case 0x3F03:
                        palette = palettes.getBackgroundPalette(0);
                        palette.colorC = value;
                        palettes.setBackgroundPalette(0, palette);

                    case 0x3F05:
                        palette = palettes.getBackgroundPalette(1);
                        palette.colorA = value;
                        palettes.setBackgroundPalette(1, palette);
                    case 0x3F06:
                        palette = palettes.getBackgroundPalette(1);
                        palette.colorB = value;
                        palettes.setBackgroundPalette(1, palette);
                    case 0x3F07:
                        palette = palettes.getBackgroundPalette(1);
                        palette.colorC = value;
                        palettes.setBackgroundPalette(1, palette);

                    case 0x3F09:
                        palette = palettes.getBackgroundPalette(2);
                        palette.colorA = value;
                        palettes.setBackgroundPalette(2, palette);
                    case 0x3F0A:
                        palette = palettes.getBackgroundPalette(2);
                        palette.colorB = value;
                        palettes.setBackgroundPalette(2, palette);
                    case 0x3F0B:
                        palette = palettes.getBackgroundPalette(2);
                        palette.colorC = value;
                        palettes.setBackgroundPalette(2, palette);

                    case 0x3F0D:
                        palette = palettes.getBackgroundPalette(3);
                        palette.colorA = value;
                        palettes.setBackgroundPalette(3, palette);
                    case 0x3F0E:
                        palette = palettes.getBackgroundPalette(3);
                        palette.colorB = value;
                        palettes.setBackgroundPalette(3, palette);
                    case 0x3F0F:
                        palette = palettes.getBackgroundPalette(3);
                        palette.colorC = value;
                        palettes.setBackgroundPalette(3, palette);

                    case 0x3F11:
                        palette = palettes.getSpritePalette(0);
                        palette.colorA = value;
                        palettes.setSpritePalette(0, palette);
                    case 0x3F12:
                        palette = palettes.getSpritePalette(0);
                        palette.colorB = value;
                        palettes.setSpritePalette(0, palette);
                    case 0x3F13:
                        palette = palettes.getSpritePalette(0);
                        palette.colorC = value;
                        palettes.setSpritePalette(0, palette);

                    case 0x3F15:
                        palette = palettes.getSpritePalette(1);
                        palette.colorA = value;
                        palettes.setSpritePalette(1, palette);
                    case 0x3F16:
                        palette = palettes.getSpritePalette(1);
                        palette.colorB = value;
                        palettes.setSpritePalette(1, palette);
                    case 0x3F17:
                        palette = palettes.getSpritePalette(1);
                        palette.colorC = value;
                        palettes.setSpritePalette(1, palette);

                    case 0x3F19:
                        palette = palettes.getSpritePalette(2);
                        palette.colorA = value;
                        palettes.setSpritePalette(2, palette);
                    case 0x3F1A:
                        palette = palettes.getSpritePalette(2);
                        palette.colorB = value;
                        palettes.setSpritePalette(2, palette);
                    case 0x3F1B:
                        palette = palettes.getSpritePalette(2);
                        palette.colorC = value;
                        palettes.setSpritePalette(2, palette);

                    case 0x3F1D:
                        palette = palettes.getSpritePalette(3);
                        palette.colorA = value;
                        palettes.setSpritePalette(3, palette);
                    case 0x3F1E:
                        palette = palettes.getSpritePalette(3);
                        palette.colorB = value;
                        palettes.setSpritePalette(3, palette);
                    case 0x3F1F:
                        palette = palettes.getSpritePalette(3);
                        palette.colorC = value;
                        palettes.setSpritePalette(3, palette);

                    default: break;
                }
                break;
        }
        std::cout << "PPU Write @ $" << makeHex(address) << " is unimplemented!"
                  << " Value: " << (int)value << " Region: " << regionName(mappedAddress.region) << std::endl;
    }
    void PPUMemory::setAddress(Address address, Address value) {
        Byte loByte = lo(value), hiByte = hi(value);
        setByte(address, loByte);
        setByte(address + (Address)1, hiByte);
    }

    PPUMemory::PPUMemory(ROM* nRom) {
        rom = nRom;

        switch (rom->header.getMirroring()) {
            case ROMHeader::Mirroring::Vertical:
                nameTable0 = &realNameTable0;
                nameTable1 = &realNameTable1;
                nameTable2 = &realNameTable0;
                nameTable3 = &realNameTable1;
                break;
            case ROMHeader::Mirroring::Horizontal:
                nameTable0 = &realNameTable0;
                nameTable1 = &realNameTable0;
                nameTable2 = &realNameTable1;
                nameTable3 = &realNameTable1;
                break;
        }
    }
}