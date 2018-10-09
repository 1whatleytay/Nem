//
// Created by Taylor Whatley on 2018-10-06.
//

#include <SDL.h>

#include "NemWindow.h"

#include <iostream>

int main(int count, char** args) {
    if (count < 1) {
        std::cout << "Too few arguments." << std::endl;
        return 0;
    }

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cout << "Could not initialize SDL" << std::endl;
        return 0;
    }

    NemSDL::NemWindow window = NemSDL::NemWindow(string(args[1]));
    //window.exec();

//    SDL_Quit();
}