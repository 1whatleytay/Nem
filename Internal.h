//
// Created by Taylor Whatley on 2018-09-28.
//

#ifndef NEM_INTERNAL_H
#define NEM_INTERNAL_H

#include <string>
#include <vector>
#include <functional>

using std::string;
using std::vector;

typedef uint8_t Byte;
typedef uint16_t Address;

#define kilobyte(a) a * 1024

#define MARIO_8057
#define FORCE_ENTRY 0xc000
#define MIRROR_ROM
#define RESET_NMI
//#define PRINT_INSTRUCTIONS
#define RTI_PC_SET

namespace Nem {
    inline Byte lo(Address value) { return (Byte)(value & 0xff); }
    inline Byte hi(Address value) { return (Byte)((value >> 8) & 0xff); }
    inline Address makeAddress(Byte a, Byte b) { return (Address)(b * 0x100 + a); }
    string makeHex(int hex);
    short makeSigned(Byte value);

    Address putByte(bool hi, Address address, Byte write);
}

#endif //NEM_INTERNAL_H
