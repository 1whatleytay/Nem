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

//#define NULL_CONTROLLER_COMPLAIN
//#define MARIO_8057
//#define FORCE_ENTRY 0x8000
//#define RTI_MINUS_ONE
//#define PRINT_INSTRUCTIONS
//#define CPU_ONLY
#define FORCE_SPRITE_ZERO_HIT
#define NMI_RESET
#define NO_AUDIO

//#define PROFILE_CPU
//#define PROFILE_DETECT_INFINITE_LOOPS

namespace Nem {
    enum Direction {
        Vertical,
        Horizontal,
    };

    bool getDebugFlag();
    Address getDebugPC();

    inline Byte lo(Address value) { return (Byte)(value & 0xff); }
    inline Byte hi(Address value) { return (Byte)((value >> 8) & 0xff); }
    inline Address makeAddress(Byte a, Byte b) { return (Address)(b * 0x100 + a); }
    string makeHex(int hex);
    string makeBin(int bin);
    short makeSigned(Byte value);

    Address putByte(bool hi, Address address, Byte write);
}

#endif //NEM_INTERNAL_H
