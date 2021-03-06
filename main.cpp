//
// Created by Taylor Whatley on 2018-10-06.
//

#ifdef CPU_ONLY
#include "CPU/CPU.h"
#include "Emulator.h"
#else
#include "App/App.h"
#endif

#include <iostream>

int main(int count, char** args) {
    if (count <= 1) {
        std::cout << "Too few arguments." << std::endl;
        return 0;
    }

#ifdef CPU_ONLY
    Nem::Emulator emulator = Nem::Emulator(string(args[1]));
    emulator.cpu->exec();
#else
    Nem::App app = Nem::App(string(args[1]));
    app.exec();
#endif

    return 0;
}
