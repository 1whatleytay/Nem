//
// Created by Taylor Whatley on 2018-10-06.
//

#include "NemWindow.h"

#include "../Errors.h"

//#include <OpenGL/gl.h>
#include <iostream>
#include <SDL_opengl.h>

namespace NemSDL {
    void NemWindow::handleEvent(SDL_Event& event) {
        if (event.type == SDL_QUIT) stopExec();

    }

    void NemWindow::init() {
        /* Clear our buffer with a red background */
        glClearColor ( 1.0, 0.0, 0.0, 1.0 );
        glClear ( GL_COLOR_BUFFER_BIT );
        /* Swap our back buffer to the front */
        SDL_GL_SwapWindow(window);
        /* Wait 2 seconds */
        SDL_Delay(2000);

        /* Same as above, but green */
        glClearColor ( 0.0, 1.0, 0.0, 1.0 );
        glClear ( GL_COLOR_BUFFER_BIT );
        SDL_GL_SwapWindow(window);
        SDL_Delay(2000);

        /* Same as above, but blue */
        glClearColor ( 0.0, 0.0, 1.0, 1.0 );
        glClear ( GL_COLOR_BUFFER_BIT );
        SDL_GL_SwapWindow(window);
        SDL_Delay(2000);
    }

    void NemWindow::render() {
        glClearColor(1, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        glBegin(GL_TRIANGLES);
        {
            glVertex2f(1, 1);
            glVertex2f(1, 0);
            glVertex2f(0, 0);
        }
        glEnd();
    }

    void NemWindow::exec() {
        SDL_Event event;
        while(!stopExecution) {
            while (SDL_PollEvent(&event)) handleEvent(event);

            glClearColor(1, 0, 0, 1);
            glClear(GL_COLOR_BUFFER_BIT);

            //std::cout << "Rendering..." << std::endl;

            SDL_GL_SwapWindow(window);

            const char* error = SDL_GetError();
            if (*error != '\0') std::cout << "SDL ERROR: " << error << std::endl;
        }
    }

    void NemWindow::stopExec() { stopExecution = true; }

    NemWindow::NemWindow(string pathToRom) : emulator(pathToRom) {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

//        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, true);
//        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

        window = SDL_CreateWindow("Nemulator",
                SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                512, 512,
                SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
        if (!window) throw Nem::CouldNotCreateWindowException();

        context = SDL_GL_CreateContext(window);

        SDL_GL_SetSwapInterval(1);

        //init();
        exec();
    }

    NemWindow::~NemWindow() {
        SDL_GL_DeleteContext(context);
        SDL_DestroyWindow(window);
    }
}