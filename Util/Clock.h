//
// Created by Taylor Whatley on 2018-10-06.
//

#ifndef NEM_CLOCK_H
#define NEM_CLOCK_H

#include <functional>

namespace Nem {
    class Clock {
        long long lastTime;
        long long cpuTick = 0;
        long long ppuTick = 0;
    public:
        void waitTicks();

        Clock();
    };
}


#endif //NEM_CLOCK_H
