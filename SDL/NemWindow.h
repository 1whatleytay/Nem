//
// Created by Taylor Whatley on 2018-10-06.
//

#ifndef NEM_NEMWINDOW_H
#define NEM_NEMWINDOW_H

#include <SDL.h>

#include "../Nem.h"

namespace NemSDL {
    class NemWindow {
        SDL_Window* window;
        SDL_GLContext context;

        volatile bool stopExecution = false;

        Nem::Emulator emulator;

        void handleEvent(SDL_Event& event);

        void init();
        void render();
    public:
        void exec();
        void stopExec();

        explicit NemWindow(string pathToRom);
        ~NemWindow();
    };
}


#endif //NEM_NEMWINDOW_H
