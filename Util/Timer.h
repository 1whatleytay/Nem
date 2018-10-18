//
// Created by Taylor Whatley on 2018-09-29.
//

#ifndef NEM_TIMER_H
#define NEM_TIMER_H

namespace Nem {
    class Timer {
        long long lastTime;
    public:
        long long interval;

        bool ready();

        Timer(long long nInterval);
    };
}


#endif //NEM_TIMER_H
