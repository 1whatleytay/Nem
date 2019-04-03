//
// Created by Taylor Whatley on 2018-10-17.
//

#include "Profiler.h"

#include "Disassembler.h"
#include "../CPU/CPU.h"
#include "../Util/Clock.h"

#include <iostream>

namespace Nem {
    void Profiler::executionAnalysis(const DisInst &inst) {
        if (config.breakpoints.doProfile) {
            if (std::find(config.breakpoints.breakpoints.begin(), config.breakpoints.breakpoints.end(),
                    inst.registers.programCounter) != config.breakpoints.breakpoints.end()) {
                std::cout << "[Breakpoint at " << makeHex(inst.registers.programCounter) << "]" << std::endl;
                // Breakpoint Next Line
                message(Breakpoint);
            }
        }
        if (config.executionAnalysis.doProfile) {
            int bidIndex = -1;
            if (config.executionAnalysis.executionMemorySizeLive >= config.executionAnalysis.executionMemorySize) {
                int bid = INT_MAX;
                for (int a = 0; a < config.executionAnalysis.executionMemorySizeLive; a++) {
                    if (config.executionAnalysis.executionMemory[a].lastReferred < bid) {
                        bid = config.executionAnalysis.executionMemory[a].lastReferred;
                        bidIndex = a;
                    }
                }
            }
            bool wrongOP = false;
            int findIndex = -1;
            for (int a = 0; a < config.executionAnalysis.executionMemorySizeLive; a++) {
                if (config.executionAnalysis.executionMemory[a].pc == inst.registers.programCounter) {
                    findIndex = a;
                    wrongOP = config.executionAnalysis.executionMemory[a].opCode != inst.code;
                    break;
                }
            }
            if (findIndex == -1) {
                InstructionOccurrences occurrences;
                occurrences.pc = inst.registers.programCounter;
                occurrences.opCode = inst.code;

                if (bidIndex == -1) {
                    findIndex = config.executionAnalysis.executionMemorySizeLive;
                    config.executionAnalysis.executionMemory[
                            config.executionAnalysis.executionMemorySizeLive] = occurrences;
                    config.executionAnalysis.executionMemorySizeLive++;
                } else {
                    findIndex = bidIndex;
                    config.executionAnalysis.executionMemory[findIndex] = occurrences;
                }
            }

            if (wrongOP) {
                InstructionOccurrences occurrences;
                occurrences.pc = inst.registers.programCounter;
                occurrences.opCode = inst.code;
                config.executionAnalysis.executionMemory[findIndex] = occurrences;
            }

            config.executionAnalysis.executionMemory[findIndex].occurrences++;
            config.executionAnalysis.executionMemory[findIndex].lastReferred =
                    config.executionAnalysis.currentInstruction;

            config.executionAnalysis.currentInstruction++;

            if (config.loopDetection.doProfile) {
                if (config.executionAnalysis.executionMemory[findIndex].occurrences >=
                    config.loopDetection.minimumExecutionCount &&
                    !config.executionAnalysis.executionMemory[findIndex].marked) {
                    setDebugFlag("profiler-loop-detected", true);

                    std::cout << "[Profiler Detected Loop]" << std::endl;
                    std::cout << inst.toString() << std::endl;
                    config.executionAnalysis.executionMemory[findIndex].marked = true;
                }
            }
        }
    }

#ifdef NEM_PROFILE_THREADED
    void Profiler::runExecutionAnalysis() {
        while (!config.executionAnalysis.stopAnalysis) {
            while (!config.executionAnalysis.processQueue.empty()) {
                config.executionAnalysis.processQueueMutex.lock();
                DisInst inst = config.executionAnalysis.processQueue.front();
                config.executionAnalysis.processQueue.pop();
                config.executionAnalysis.processQueueMutex.unlock();
                executionAnalysis(inst);
            }
        }
    }
#endif
    void Profiler::printTrail() {
        if (config.trail.doProfile) {
            std::cout << "[Profiler Trail]" << std::endl;
            while (!config.trail.trailQueue.empty()) {
                DisInst inst = config.trail.trailQueue.front();
                std::cout << inst.toString() << std::endl;
                config.trail.trailQueue.pop();
            }
            if (config.trail.listMemory) {
                std::cout << "[Profiler Trail List]" << std::endl;
                cpu->memory.list(cpu->registers.programCounter - (Address)config.trail.listBytesBefore,
                                 (Address)config.trail.listBytesAfter +
                                 (Address)config.trail.listBytesBefore + (Address)1);
            }
        }
    }

    void Profiler::message(Nem::Profiler::ProfilerEvent event) {
        if (!config.doProfile || noError) return;

        switch (event) {
            case Breakpoint:
                std::cout << "[Profiler Breakpoint @" << makeHex(cpu->registers.programCounter) << "]" << std::endl;
                printTrail();
                break;
            case OutOfBounds:
                std::cout << "[Profiler OutOfBounds @" << makeHex(cpu->registers.programCounter) << "]" << std::endl;
                printTrail();
                break;
            case IRQ:
            case NMI:
                if (config.nmiRtiMatching.doProfile && config.nmiRtiMatching.nmiOpen) {
                    std::cout << "[Profile NMI Called While NMI is Open @"
                              << makeHex(cpu->registers.programCounter) << "]" << std::endl;
                } else config.nmiRtiMatching.nmiOpen = true;
                break;
        }
    }

    void Profiler::analyzeStep() {
        if (!config.doProfile) return;

        noError = true;
        Byte fetch[3];
        for (Address a = 0; a < 3; a++) fetch[a] = cpu->memory.getByte(cpu->registers.programCounter + a, false);

        DisInst inst = DisInst(fetch, cpu->registers, cpu->cycles);

        if (config.printInstructions.doProfile) {
            if (config.printInstructions.binary) {
                vector<Byte> data = inst.toBinary();
                config.printInstructions.outFile.write((char *) &data[0], data.size());
            } else {
                config.printInstructions.outFile << inst.toString() << "\n";
            }
        }

        if (config.trail.doProfile) {
            config.trail.trailQueue.push(inst);
            if (config.trail.trailQueue.size() > config.trail.trailLength) {
                config.trail.trailQueue.pop();
            }
        }

        if (config.nmiRtiMatching.doProfile && inst.code == 0x40) {
            if (!config.nmiRtiMatching.nmiOpen) {
                std::cout << "[Profile RTI Called While NMI is Closed @"
                          << makeHex(cpu->registers.programCounter) << "]" << std::endl;
            } else config.nmiRtiMatching.nmiOpen = false;
        }

#ifdef NEM_PROFILE_THREADED
        config.executionAnalysis.processQueueMutex.lock();
        config.executionAnalysis.processQueue.push(inst);
        config.executionAnalysis.processQueueMutex.unlock();
#else
        executionAnalysis(inst);
#endif
        noError = false;
    }

    Profiler::Profiler(Nem::CPU *nCpu) : cpu(nCpu) {
        if (config.printVectors.doProfile) {
            std::cout
            << "[IRQ: " << makeHex(cpu->memory.getIRQVector())
            << " RESET: " << makeHex(cpu->memory.getResetVector())
            << " NMI: " << makeHex(cpu->memory.getNMIVector()) << "]" << std::endl;
        }
        if (config.printInstructions.doProfile) {
            config.printInstructions.outFile.open(config.printInstructions.fileName,
                    config.printInstructions.binary ? std::ios::trunc | std::ios::binary : std::ios::trunc);
        }
        if (config.executionAnalysis.doProfile) {
            config.executionAnalysis.executionMemory = vector<InstructionOccurrences>(
                    (unsigned long)config.executionAnalysis.executionMemorySize);
        }

#ifdef NEM_PROFILE_THREADED
        config.executionAnalysis.analysisThread = new std::thread(&Profiler::runExecutionAnalysis, this);
#endif
    }
    Profiler::~Profiler() {
        if (config.printInstructions.doProfile) config.printInstructions.outFile.close();

#ifdef NEM_PROFILE_THREADED
        config.executionAnalysis.stopAnalysis = true;
        if (config.executionAnalysis.analysisThread) config.executionAnalysis.analysisThread->join();
        delete config.executionAnalysis.analysisThread;
#endif
    }
}
