//
// Created by Taylor Whatley on 2018-10-17.
//

#ifndef NEM_PROFILER_H
#define NEM_PROFILER_H

namespace Nem {
    class CPU;

    class Profiler {
        // Analyze where most of the CPU time is being spent.
        // Detect infinite loops

        CPU* cpu;
    public:
        void analyzeStep();

        explicit Profiler(CPU* nCpu);
    };
}

#endif //NEM_PROFILER_H
