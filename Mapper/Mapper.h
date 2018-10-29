//
// Created by Taylor Whatley on 2018-10-11.
//

#ifndef NEM_MAPPER_H
#define NEM_MAPPER_H

#include "../Internal.h"

#include <functional>

namespace Nem {
    class ROM;
    class ROMHeader;

    class Mapper {
    protected:
        int index;
        ROM* rom;

        vector<Byte> chrRAM;

        int getBankReference(Address address);
    public:
        bool ppuNeedsRefresh = false;

        string getName();

        virtual Direction getMirroring();

        virtual Byte getRAMByte(Address index) = 0;
        virtual void setRAMByte(Address index, Byte value) = 0;

        virtual Byte getPRGByte(Address index) = 0;
        virtual void setPRGByte(Address address, Byte value) = 0;

        virtual Byte getCHRByte(Address index) = 0;
        virtual void setCHRByte(Address address, Byte value) = 0;

        virtual Address getNMIVector();
        virtual Address getResetVector();
        virtual Address getIRQVector();

        explicit Mapper(int nIndex, ROM* nRom);
        virtual ~Mapper() = default;
    };

    extern const std::function<Mapper* (ROM* rom)> mappers[16];
    extern const string mapperNames[16];
}

#endif //NEM_MAPPER_H
