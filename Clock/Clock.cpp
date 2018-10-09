//
// Created by Taylor Whatley on 2018-10-06.
//

#include "Clock.h"

#include <chrono>

namespace Nem {
    long long currentTime() {
        return std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
    }

    long long hertz(long long time) {
        return (long long)((double)time / 1000000.0 * 21477272.0);
    }

    bool Clock::cpuReady(long long cycles) {
        long long hz = hertz(currentTime());
        long long difference = hz - cpuLastTime;
        long long cyclesInHz = cycles * 12;
        if (difference >= cyclesInHz) {
            cpuLastTime += cyclesInHz;
            return true;
        }
        return false;
    }

    bool Clock::ppuReady(long long cycles) {
        long long hz = hertz(currentTime());
        long long difference = hz - ppuLastTime;
        long long cyclesInHz = cycles * 4;
        if (difference >= cyclesInHz) {
            ppuLastTime += cyclesInHz;
            return true;
        }
        return false;
    }

    Clock::Clock() : cpuLastTime(hertz(currentTime())), ppuLastTime(hertz(currentTime())) { }

    bool Stopwatch::hasBeen(long long milliseconds) {
        return lastTime + milliseconds <= currentTime() / 1000;
    }

    void Stopwatch::start() {
        lastTime = currentTime() / 1000;
        lap = 0;
    }
    long long Stopwatch::stop() {
        return currentTime() / 1000 - lastTime;
    }
}