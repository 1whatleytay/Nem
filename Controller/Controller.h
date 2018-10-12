//
// Created by Taylor Whatley on 2018-10-09.
//

#ifndef NEM_CONTROLLER_H
#define NEM_CONTROLLER_H

#include "../Internal.h"

namespace Nem {
    enum Buttons {
        ButtonA      = 0b10000000,
        ButtonB      = 0b01000000,
        ButtonSelect = 0b00100000,
        ButtonStart  = 0b00010000,
        ButtonUp     = 0b00001000,
        ButtonDown   = 0b00000100,
        ButtonLeft   = 0b00000010,
        ButtonRight  = 0b00000001,
    };

    class ControllerInterface {
    public:
        virtual Byte read() = 0;
        virtual void write(Byte data) = 0;

        virtual ~ControllerInterface() = default;
    };

    class NullController : public ControllerInterface {
    public:
        Byte read() override;
        void write(Byte data) override;
    };

    class EditController: public ControllerInterface {
        int readId = 0;

        bool strobe = false;

        Byte backButton = 0;
        Byte frontButton = 0;
    public:
        void press(Byte buttons);
        void release(Byte buttons);

        Byte read() override;
        void write(Byte data) override;
    };
}

#endif //NEM_CONTROLLER_H
