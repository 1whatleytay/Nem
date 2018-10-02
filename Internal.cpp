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

    Address putByte(bool hi, Address address, Byte write) {
        if (hi) return (address & (Address)0x00ff) | (Address)write << 8;
        else return (address & (Address)0xff00) | (Address)write;
    }
}