//
// Created by Taylor Whatley on 2018-10-06.
//

#include "Clock.h"

#include "Stopwatch.h"

#include <chrono>
#include <thread>
#include <iostream>

namespace Nem {
    void noTick(long long) { }

    long long currentTimeNano() {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    }

    long long clockCycle12 = 559;
    long long clockCycleSlow = clockCycle12 * 2;

    void Clock::waitTicks() {
        long long waitTime = clockCycle12;
        long long finishedTime = lastTime + waitTime;
        long long ctNano = currentTimeNano();

        while (ctNano < finishedTime) {
            ctNano = currentTimeNano();
        }
        lastTime += waitTime;
    }

    Clock::Clock() : lastTime(currentTimeNano()) { }
}