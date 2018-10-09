//
// Created by Taylor Whatley on 2018-10-06.
//

#include "Display/Display.h"

#include <iostream>

int main(int count, char** args) {
    if (count < 1) {
        std::cout << "Too few arguments." << std::endl;
        return 0;
    }

    Nem::Display display = Nem::Display(string(args[1]));

    display.exec();
}