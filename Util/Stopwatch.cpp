//
// Created by Taylor Whatley on 2018-11-27.
//

#include "Stopwatch.h"

#include <chrono>

namespace Nem {
    long long Stopwatch::currentTime() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    }

    bool Stopwatch::hasBeen(long long milliseconds) {
        return lastTime + milliseconds < currentTime();
    }

    void Stopwatch::start() {
        lastTime = currentTime();
        lap = 0;
    }

    long long Stopwatch::stop() {
        return currentTime() - lastTime;
    }

    long long Stopwatch::reset() {
        long long value = stop();
        start();
        return value;
    }

    Stopwatch::Stopwatch() { start(); }

    long long NanoStopwatch::currentTime() {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    }
}