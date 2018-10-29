//
// Created by Taylor Whatley on 2018-09-28.
//

#include "Internal.h"

#include <sstream>
#include <iomanip>

namespace Nem {
    short makeSigned(Byte value) {
        if (value <= 128) return (short)value;
        else return (short)value - (short)256;
    }

    string makeHex(int hex) {
        std::stringstream stream;
        stream << std::hex << hex;
        return stream.str();
    }

    string makeHexZeroed(int hex, int padding) {
        string old = makeHex(hex);
        return std::string(padding - old.length(), '0') + old;
    }

    string makeHex(Byte hex) {
        return makeHexZeroed(hex, 2);
    }
    string makeHex(Address hex) {
        return makeHexZeroed(hex, 4);
    }

    string makeBin(Byte bin) {
        return std::bitset<sizeof(Byte) * 8>(bin).to_string();
    }
    string makeBin(Address bin) {
        return std::bitset<sizeof(Address) * 8>(bin).to_string();
    }

    string makeBin(int bin) {
        return std::bitset<sizeof(int) * 8>(bin).to_string();
    }

    Address putByte(bool hi, Address address, Byte write) {
        if (hi) return (address & (Address)0x00ff) | (Address)write << 8;
        else return (address & (Address)0xff00) | (Address)write;
    }
}