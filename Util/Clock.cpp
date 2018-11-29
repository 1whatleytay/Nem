//
// Created by Taylor Whatley on 2018-10-06.
//

#include "Clock.h"

#include <chrono>
#include <thread>
#include <iostream>

namespace Nem {
    void noTick(long long) { }

    long long currentTimeNano() {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    }

    long long ticksToNanoseconds(double ticks) {
        return (long long)((ticks / 21477272.0) * 1000000000);
    }

    void Clock::waitTicks(long long ticks) {
        long long finishedTime = lastTime + ticksToNanoseconds(ticks);
        long long ctNano = currentTimeNano();
        while (ctNano < finishedTime) {
#ifdef YIELD_ON_TICK
            std::this_thread::yield();
#endif
            ctNano = currentTimeNano();
        }
        lastTime = ctNano;
    }

    void Clock::stopExec() { stopExecution = true; }

    void Clock::exec() {
        while (!stopExecution) {
            ppuCallback(ppuTick++);
            ppuCallback(ppuTick++);
            ppuCallback(ppuTick++);
            cpuCallback(cpuTick++);
            waitTicks(12);
        }
    }

    Clock::Clock() : lastTime(currentTimeNano()) { }
    Clock::~Clock() {
        cpuCallback(-1);
        ppuCallback(-1);
    }
}