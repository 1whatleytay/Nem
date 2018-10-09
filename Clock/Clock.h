//
// Created by Taylor Whatley on 2018-10-06.
//

#ifndef NEM_CLOCK_H
#define NEM_CLOCK_H

namespace Nem {
    class Clock {
        long long cpuLastTime;
        long long ppuLastTime;
    public:

        bool cpuReady(long long cycles);
        bool ppuReady(long long cycles);

        Clock();
    };

    class Stopwatch {
        long long lastTime = 0;
    public:
        int lap;

        bool hasBeen(long long milliseconds);

        void start();
        long long stop();
    };
}


#endif //NEM_CLOCK_H
