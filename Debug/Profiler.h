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

        bool loopDetectionHasMarked = false;
    };

    struct ProfilerConfigPrintVectors {
        bool doProfile = true;
    };

    struct ProfilerConfigPrintInstructions {
        bool doProfile = false;
        bool binary = true;
        string fileName = "/Users/desgroup/Desktop/nem.log.dat";
        std::ofstream outFile;
    };

    struct ProfilerConfigTrail {
        bool doProfile = true;
        int trailLength = 100;
        std::queue<DisInst> trailQueue;
        bool listMemory = false;
        int listBytesBefore = 20;
        int listBytesAfter = 20;
    };

    struct ProfilerConfigExecutionAnalysis {
        bool doProfile = false;
        int executionMemorySize = 2000;
        int currentInstruction = 0;
        int executionMemorySizeLive = 0;
        InstructionOccurrences* executionMemory;
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

    struct ProfilerConfig {
        bool doProfile = true;
        ProfilerConfigPrintVectors printVectors;
        ProfilerConfigPrintInstructions printInstructions;
        ProfilerConfigTrail trail;
        ProfilerConfigExecutionAnalysis executionAnalysis;
        ProfilerConfigLoopDetection loopDetection;
    };

    class Profiler {
        // Analyze where most of the CPU time is being spent.
        // Detect infinite loops
        CPU* cpu;
        bool noBreakpoint = false;

        void executionAnalysis(DisInst inst);

#ifdef NEM_PROFILE_THREADED
        void runExecutionAnalysis();
#endif
    public:
        ProfilerConfig config;

        void breakpoint();
        void analyzeStep();

        explicit Profiler(CPU* nCpu);
        ~Profiler();
    };
}

#endif //NEM_PROFILER_H
