//
// Created by Taylor Whatley on 2018-09-28.
//

#ifndef NEM_INTERNAL_H
#define NEM_INTERNAL_H

#include <string>
#include <vector>

using std::string;
using std::vector;

typedef uint8_t Byte;
typedef uint16_t Address;

#define kilobyte(a) a * 1024

#define NEM_PROFILE
//#define NEM_PROFILE_THREADED
//#define NEM_PROFILE_ANALYSIS_FLAG

namespace Nem {
    bool getDebugFlag(string name);
    void setDebugFlag(string name, bool value);

    enum Direction {
        Vertical,
        Horizontal,
    };

    Address getDebugPC();

    inline Byte lo(Address value) { return (Byte)(value & 0xff); }
    inline Byte hi(Address value) { return (Byte)((value >> 8) & 0xff); }
    inline Address makeAddress(Byte a, Byte b) { return (Address)(b * 0x100 + a); }
    string makeHex(Byte hex);
    string makeHex(Address hex);
    string makeHex(int hex);
    string makeBin(Byte bin);
    string makeBin(Address bin);
    string makeBin(int bin);
    short makeSigned(Byte value);

    Address putByte(bool hi, Address address, Byte write);
}

#endif //NEM_INTERNAL_H
