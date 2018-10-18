//
// Created by Taylor Whatley on 2018-10-17.
//

#include "Profiler.h"

#include "../CPU/CPU.h"

namespace Nem {
    void Profiler::analyzeStep() {
        Address pc = cpu->registers->programCounter + (Address)1;
    }

    Profiler::Profiler(Nem::CPU *nCpu) : cpu(nCpu) { }
}