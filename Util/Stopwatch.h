//
// Created by Taylor Whatley on 2018-11-27.
//

#ifndef NEM_STOPWATCH_H
#define NEM_STOPWATCH_H

namespace Nem {
    class Stopwatch {
        static long long currentTimeMillis();

        long long lastTime = 0;
    public:
        int lap = 0;

        bool hasBeen(long long milliseconds);

        void start();
        long long stop();
    };

    class NanoStopwatch {
        static long long currentTimeNano();

        long long lastTime = 0;
    public:
        int lap = 0;

        bool hasBeen(long long nanoseconds);

        void start();
        long long stop();
    };
}

#endif //NEM_STOPWATCH_H
