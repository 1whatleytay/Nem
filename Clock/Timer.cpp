//
// Created by Taylor Whatley on 2018-09-29.
//

#include "Timer.h"

#include <chrono>

namespace Nem {
    long long currentTime() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
    }

    bool Timer::ready() {
        long long current = currentTime();
        if (lastTime + interval <= current) {
            lastTime = current;
            return true;
        }
        return false;
    }

    Timer::Timer(long long nInterval) : interval(nInterval), lastTime(currentTime()) { }
}