//
// Created by Taylor Whatley on 2018-10-16.
//

#include "App.h"

#include "../ROM/ROM.h"
#include "../CPU/CPU.h"
#include "../PPU/PPU.h"
#include "../Util/Clock.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <iostream>

namespace Nem {
    void App::loadBindings() {
        display.mainControllerBindings.insert({GLFW_KEY_UP, Buttons::ButtonUp});
        display.mainControllerBindings.insert({GLFW_KEY_DOWN, Buttons::ButtonDown});
        display.mainControllerBindings.insert({GLFW_KEY_LEFT, Buttons::ButtonLeft});
        display.mainControllerBindings.insert({GLFW_KEY_RIGHT, Buttons::ButtonRight});
        display.mainControllerBindings.insert({GLFW_KEY_X, Buttons::ButtonA});
        display.mainControllerBindings.insert({GLFW_KEY_Z, Buttons::ButtonB});
        display.mainControllerBindings.insert({GLFW_KEY_ENTER, Buttons::ButtonStart});
        display.mainControllerBindings.insert({GLFW_KEY_SPACE, Buttons::ButtonSelect});

        display.secondControllerBindings.insert({GLFW_KEY_W, Buttons::ButtonUp});
        display.secondControllerBindings.insert({GLFW_KEY_S, Buttons::ButtonDown});
        display.secondControllerBindings.insert({GLFW_KEY_A, Buttons::ButtonLeft});
        display.secondControllerBindings.insert({GLFW_KEY_D, Buttons::ButtonRight});
        display.secondControllerBindings.insert({GLFW_KEY_K, Buttons::ButtonA});
        display.secondControllerBindings.insert({GLFW_KEY_L, Buttons::ButtonB});
        display.secondControllerBindings.insert({GLFW_KEY_E, Buttons::ButtonStart});
        display.secondControllerBindings.insert({GLFW_KEY_R, Buttons::ButtonSelect});
    }

    void App::exec() {
        cpuThread = new std::thread(&CPU::exec, cpu);

        display.exec();
        std::cout << "Press that statement." << std::endl;
    }

    App::App(string pathToRom) : Emulator(std::move(pathToRom)), display(ppu) {
        std::cout << rom->getRomInfo() << std::endl;

        loadBindings();

        setController(0, &display.mainController);
        setController(1, &display.secondController);
    }

    App::~App() {
        cpu->stopExec();
        if (cpuThread) cpuThread->join();
        delete cpuThread;
    }
}