//
// Created by Taylor Whatley on 2018-11-27.
//

#include "Stopwatch.h"

#include <chrono>

namespace Nem {
    long long Stopwatch::currentTimeMillis() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    }

    long long NanoStopwatch::currentTimeNano() {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    }

    bool NanoStopwatch::hasBeen(long long nanoseconds) {
        return lastTime + nanoseconds < currentTimeNano();
    }

    void NanoStopwatch::start() {
        lastTime = currentTimeNano();
        lap = 0;
    }

    long long NanoStopwatch::stop() {
        return currentTimeNano() - lastTime;
    }

    bool Stopwatch::hasBeen(long long milliseconds) {
        return lastTime + milliseconds < currentTimeMillis();
    }

    void Stopwatch::start() {
        lastTime = currentTimeMillis();
        lap = 0;
    }

    long long Stopwatch::stop() {
        return currentTimeMillis() - lastTime;
    }


}