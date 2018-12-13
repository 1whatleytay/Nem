//
// Created by Taylor on 2018-12-10.
//

#include "App.h"

#include <iostream>

namespace Nem {
    void Display::exec() {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();

            glClearColor(1, 1, 0, 1);
            glClear(GL_COLOR_BUFFER_BIT);

            glfwSwapBuffers(window);
        }
    }

    Display::Display(Nem::PPU *ppu) : ppu(ppu) {
        if (!glfwInit()) throw CouldNotCreateWindowException();

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);

        window = glfwCreateWindow(windowWidth, windowHeight, "Nem", nullptr, nullptr);
        if (!window) throw CouldNotCreateWindowException();

        glfwMakeContextCurrent(window);
        glfwSwapInterval(0);

#ifdef _WIN32
        if (gl3wInit() != GL3W_OK) throw CouldNotCreateWindowException();
#endif
    }

    Display::~Display() {
        glfwDestroyWindow(window);

        glfwTerminate();
    }
}