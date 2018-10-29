//
// Created by Taylor Whatley on 2018-10-06.
//

#ifndef NEM_CLOCK_H
#define NEM_CLOCK_H

namespace Nem {
    class Clock {
        long long cpuLastTime;
        long long ppuLastTime;

        bool cpuStarted = false;
        bool ppuStarted = false;

        bool cpuReady(int cycles);
        bool ppuReady(int cycles);
    public:
        void waitUntilCPUReady(int cycles);
        void waitUntilPPUReady(int cycles);

        void startCPU();
        void startPPU();

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
