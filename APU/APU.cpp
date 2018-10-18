//
// Created by Taylor Whatley on 2018-10-12.
//

#include "APU.h"

namespace Nem {
    APU::APU() {
        registers = new APURegisters();
    }

    APU::~APU() {
        delete registers;
    }
}