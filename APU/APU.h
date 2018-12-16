//
// Created by Taylor Whatley on 2018-10-12.
//

#ifndef NEM_APU_H
#define NEM_APU_H

#include "../Internal.h"

namespace Nem {
    struct APUPulse { Byte envelope = 0, sweep = 0, timer = 0, length = 0; };
    struct APUTriangle { Byte control = 0, extra = 0, timer = 0, length = 0; };
    struct APUNoise { Byte envelope = 0, extra = 0, loop = 0, length = 0; };
    struct APUDMC { Byte loop = 0, load = 0, address = 0, length = 0; };

    class APURegisters {
    public:
        APUPulse pulse1;
        APUPulse pulse2;
        APUTriangle triangle;
        APUNoise noise;
        APUDMC dmc;

        Byte channels = 0;
        Byte frameCounter = 0;
    };

    class APU {
    public:
        APURegisters registers;
    };
}


#endif //NEM_APU_H
