//
// Created by Taylor Whatley on 2018-10-09.
//

#include "Controller.h"

#include <iostream>

namespace Nem {
    Byte NullController::read() {
#ifdef NULL_CONTROLLER_COMPLAIN
        std::cout << "Reading from NullController." << std::endl;
#endif
        return 0;
    }

    void NullController::write(Byte data) {
#ifdef NULL_CONTROLLER_COMPLAIN
        std::cout << "Writing to NullController <- " << (int)data << "." << std::endl;
#endif
    }

    void EditController::press(Byte buttons) {
        backButton |= buttons;

        if (strobe) frontButton = backButton;
    }
    void EditController::release(Byte buttons) {
        backButton &= ~buttons;

        if (strobe) frontButton = backButton;
    }

    Byte EditController::read() {
        int num = readId % 8;
        Byte mask = (Byte)0b10000000 >> num;

        readId++;

        return (frontButton & mask) > 0;
    }
    void EditController::write(Byte data) {
        strobe = data;

        if (strobe) frontButton = backButton;
    }
}