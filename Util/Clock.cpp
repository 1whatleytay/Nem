//
// Created by Taylor Whatley on 2018-10-06.
//

#include "Clock.h"

#include <chrono>
#include <thread>
#include <iostream>

#ifdef CPU_ONLY
#undef SYNC_CPU_PPU
#endif

namespace Nem {
    long long currentTime() {
        return std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
    }

    long long hertz(long long time) {
        return (long long)((double)time / 1000000.0 * 21477272.0);
    }

    long long millis(long long hz) {
        return (long long)((double)hz / 21377272.0 * 1000000.0);
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

    void Clock::waitUntilCPUReady(long long cycles) {
#ifdef NO_SLEEP
        while (!cpuReady(cycles)) { }
#else
        long long time = currentTime();
        long long timePassed = time - cpuLastTime;
        long long timeToWait = millis(cycles * 12) - timePassed;
        if (timeToWait > 0) std::this_thread::sleep_for(std::chrono::microseconds(timeToWait));
        cpuLastTime = time + timeToWait;
#endif
    }
    void Clock::waitUntilPPUReady(long long cycles) {
#ifdef NO_SLEEP
        while (!ppuReady(cycles)) { }
#else
        long long time = currentTime();
        long long timePassed = time - ppuLastTime;
        long long timeToWait = millis(cycles * 4) - timePassed;
        if (timeToWait > 0) std::this_thread::sleep_for(std::chrono::microseconds(timeToWait));
        ppuLastTime = time + timeToWait;
#endif
    }

    void Clock::startCPU() {
#ifdef SYNC_CPU_PPU
        cpuStarted = true;
        while (!ppuStarted) { }
#endif
#ifdef NO_SLEEP
        cpuLastTime = hertz(currentTime());
#else
        cpuLastTime = currentTime();
#endif
    }
    void Clock::startPPU() {
#ifdef SYNC_CPU_PPU
        ppuStarted = true;
        while (!cpuStarted) { }
#endif
#ifdef NO_SLEEP
        ppuLastTime = hertz(currentTime());
#else
        ppuLastTime = currentTime();
#endif
    }

    Clock::Clock() : cpuLastTime(hertz(currentTime())), ppuLastTime(hertz(currentTime())) { }

    bool Stopwatch::hasBeen(long long milliseconds) {
        return lastTime + milliseconds <= currentTime() / 1000;
    }

    void Stopwatch::start() {
        lastTime = currentTime();
        lap = 0;
    }
    long long Stopwatch::stop() {
        return currentTime() - lastTime;
    }
}