//
// Created by Taylor Whatley on 2018-11-27.
//

#ifndef NEM_STOPWATCH_H
#define NEM_STOPWATCH_H

namespace Nem {
    class Stopwatch {
        long long lastTime = 0;
    protected:
        virtual long long currentTime();
    public:
        int lap = 0;

        bool hasBeen(long long time);

        void start();
        long long stop();
        long long reset();

        Stopwatch();
    };

    class NanoStopwatch : public Stopwatch {
    protected:
        long long currentTime() override;
    public:
        NanoStopwatch() = default;
    };
}

#endif //NEM_STOPWATCH_H
