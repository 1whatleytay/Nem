//
// Created by Taylor Whatley on 2018-10-06.
//

#ifndef NEM_CLOCK_H
#define NEM_CLOCK_H

#include <functional>

namespace Nem {
    typedef std::function<void(long long tick)> TickCallback;

    void noTick(long long);

    class Clock {
        volatile bool stopExecution = false;

        long long lastTime;
        long long cpuTick = 0;
        long long ppuTick = 0;

        void waitTicks();
    public:
        void stopExec();
        void exec();

        TickCallback cpuCallback = noTick, ppuCallback = noTick;

        Clock();
        ~Clock();
    };
}


#endif //NEM_CLOCK_H
