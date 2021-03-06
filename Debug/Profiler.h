//
// Created by Taylor Whatley on 2018-10-17.
//

#ifndef NEM_PROFILER_H
#define NEM_PROFILER_H

#include "../Internal.h"

#include "Disassembler.h"

#include <queue>
#include <fstream>

#ifdef NEM_PROFILE_THREADED
#include <thread>
#endif

namespace Nem {
    class CPU;

    struct InstructionOccurrences {
        Address pc = 0x0000;
        Byte opCode = 0x00;

        int occurrences = 0;
        int lastReferred = 0;

        bool marked = false;
    };

    struct ProfilerConfigPrintVectors {
        bool doProfile = false;
    };

    struct ProfilerConfigPrintInstructions {
        bool doProfile = false;
        bool binary = false;
        string fileName = "/Users/desgroup/Desktop/nem.log.txt";
        std::ofstream outFile;
    };

    struct ProfilerConfigTrail {
        bool doProfile = false;
        int trailLength = 100;
        std::queue<DisInst> trailQueue;
        bool listMemory = false;
        int listBytesBefore = 20;
        int listBytesAfter = 20;
    };

    struct ProfilerConfigNMIRTIMatching {
        bool doProfile = false;
        bool nmiOpen = false;
    };

    struct ProfilerConfigExecutionAnalysis {
        bool doProfile = false;
        int executionMemorySize = 2000;
        int currentInstruction = 0;
        int executionMemorySizeLive = 0;
        vector<InstructionOccurrences> executionMemory;
#ifdef NEM_PROFILE_THREADED
        volatile bool stopAnalysis = false;
        std::queue<DisInst> processQueue;
        std::mutex processQueueMutex;
        std::thread* analysisThread = nullptr;
#endif
    };

    struct ProfilerConfigLoopDetection {
        bool doProfile = false;
        int minimumExecutionCount = 1000;
    };

    struct ProfilerConfigBreakpoints {
        bool doProfile = false;
        vector<Address> breakpoints = {
                0x817e,
                0x816f
        };
    };

    struct ProfilerConfig {
        bool doProfile = true;
        ProfilerConfigPrintVectors printVectors;
        ProfilerConfigPrintInstructions printInstructions;
        ProfilerConfigTrail trail;
        ProfilerConfigNMIRTIMatching nmiRtiMatching;
        ProfilerConfigExecutionAnalysis executionAnalysis;
        ProfilerConfigLoopDetection loopDetection;
        ProfilerConfigBreakpoints breakpoints;
    };

    class Profiler {
        CPU* cpu;
        bool noError;

        void executionAnalysis(const DisInst &inst);
        void printTrail();

#ifdef NEM_PROFILE_THREADED
        void runExecutionAnalysis();
#endif
    public:
        ProfilerConfig config;

        enum ProfilerEvent {
            Breakpoint,
            OutOfBounds,
            NMI,
            IRQ,
        };

        void message(ProfilerEvent event);
        void analyzeStep();

        explicit Profiler(CPU* nCpu);
        ~Profiler();
    };
}

#endif //NEM_PROFILER_H
