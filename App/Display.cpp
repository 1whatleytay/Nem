//
// Created by Taylor Whatley on 2018-10-06.
//

#include "App.h"

#include "../PPU/PPU.h"
#include "../Util/Clock.h"

#include <GLFW/glfw3.h>

#include <fstream>
#include <iostream>

namespace Nem {
    const double windowSizeMultiplier = 2.5;
    const int spriteCount = 0x2000 / 16;

    void windowKeyInput(GLFWwindow* window, int key, int scancode, int action, int mods) {
        Display* display = (Display*)glfwGetWindowUserPointer(window);
        display->keyInput(key, action);
    }

    void Display::forceShow() {
        int x, y;
        glfwGetWindowPos(window, &x, &y);
        glfwPollEvents();
        glfwSwapBuffers(window);
        glfwSetWindowPos(window, x + 1, y);
    }

    void Display::checkEdits() {
        ppu->memory->edits.mutex.lock();
        bool ppuNeedsRefresh = ppu->memory->checkNeedsRefresh();
        if (!ppu->memory->edits.nameTable.ranges.empty()) refreshNameTable();
        if (!ppu->memory->edits.oam.ranges.empty()) refreshOAM();
        if (ppuNeedsRefresh || !ppu->memory->edits.patternTable.ranges.empty()) refreshPatternTable();

        if (ppu->memory->edits.registers) refreshRegisters();
        if (ppu->memory->edits.paletteRam) refreshPaletteRam();

        ppu->memory->edits.reset();
        ppu->memory->edits.mutex.unlock();
    }

    bool Display::init() {
        if (!loadShaders()) return false;
        loadUniforms();

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glGenSamplers(1, &sampler);
        glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glSamplerParameteri(sampler, GL_TEXTURE_WRAP_R, GL_REPEAT);

        glGenTextures(1, &patternTexture);
        glActiveTexture(GL_TEXTURE0 + 0);
        glBindTexture(GL_TEXTURE_2D, patternTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, 8, spriteCount * 8,
                     0, GL_RED_INTEGER, GL_INT, nullptr);
        glBindSampler(0, sampler);
        refreshPatternTable();

        glGenTextures(1, &nameTableTexture);
        glActiveTexture(GL_TEXTURE0 + 1);
        glBindTexture(GL_TEXTURE_1D, nameTableTexture);
        glTexImage1D(GL_TEXTURE_1D, 0, GL_R32UI, kilobyte(2),
                     0, GL_RED_INTEGER, GL_INT, nullptr);
        glBindSampler(1, sampler);
        refreshNameTable();

        glGenTextures(1, &paletteTexture);
        glActiveTexture(GL_TEXTURE0 + 2);
        glBindTexture(GL_TEXTURE_1D, paletteTexture);
        glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB8, 0x40, 0, GL_RGB, GL_FLOAT, palette2C02);
        glBindSampler(2, sampler);

        glGenTextures(1, &oamTexture);
        glActiveTexture(GL_TEXTURE0 + 3);
        glBindTexture(GL_TEXTURE_1D, oamTexture);
        glTexImage1D(GL_TEXTURE_1D, 0, GL_R32UI, 4 * 64, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, nullptr);
        glBindSampler(3, sampler);
        refreshOAM();

        refreshPaletteRam();

        forceShow();

        return true;
    }

    void Display::keyInput(int key, int action) {
        if (mainControllerBindings.find(key) != mainControllerBindings.end()) {
            if (action == GLFW_PRESS) mainController.press(mainControllerBindings[key]);
            else if (action == GLFW_RELEASE) mainController.release(mainControllerBindings[key]);
        } else if (secondControllerBindings.find(key) != secondControllerBindings.end()) {
            if (action == GLFW_PRESS) secondController.press(secondControllerBindings[key]);
            else if (action == GLFW_RELEASE) secondController.release(secondControllerBindings[key]);
        }
    }

    void Display::skipCycles(long long num) {
        stop.lap += num;
        if (stop.hasBeen(1000000000)) {
            times.push_back(stop.lap);
            stop.reset();
        }
        while (processedTick + num > currentTick && currentTick != -1) { }
        processedTick += num;
    }

    void Display::exec() {
        while (!stopExecution && !glfwWindowShouldClose(window)) {
            glUseProgram(backgroundProgram);
            for (int a = 0; a < 240; a++) {
//                checkEdits();
//                glUniform1i(uniformBkgPatternTableDrawIndex,
//                        ppu->isControlSet(PPURegisters::ControlFlags::BkgPatternTable));
//                glDrawArrays(GL_TRIANGLES, (GLint) a / 8 * 32 * 6, 30 * 32 * 6);
                skipCycles(261 - (a == 0 && ppu->isMaskSet(PPURegisters::MaskFlags::ShowBKG) && frame % 2 == 1));
            }

//            checkEdits();
//            glUseProgram(spriteProgram);
//            glUniform1i(uniformSprPatternTableDrawIndex,
//                    ppu->isControlSet(PPURegisters::ControlFlags::SprPatternTable));
//            glDrawArrays(GL_TRIANGLES, 0, 64 * 6);

            skipCycles(261 + 1);
            ppu->registers->status |= PPURegisters::StatusFlags::VBlank;
            if (ppu->isControlSet(PPURegisters::ControlFlags::VBlankNMI)) ppu->postNMI();
            glfwPollEvents();
            glfwSwapBuffers(window);
            glClear(GL_COLOR_BUFFER_BIT);

            skipCycles(260 + 261 * 19 + 1);

            ppu->registers->status &= ~PPURegisters::StatusFlags::SprOverflow;
            ppu->registers->status &= ~PPURegisters::StatusFlags::VBlank;
            ppu->registers->status &= ~PPURegisters::StatusFlags::SprZeroHit;

            skipCycles(260);

            frame++;
        }
        clock->stopExec();
    }

    const string colorReset = "\033[0;m";
    const string colorRed = "\033[31;m";
    const string colorYellow = "\033[33;m";
    const string colorGreen = "\033[32;m";
    const int ticksNearby = 1000;

    void Display::calcPerformance() {
        long long lastResult = 0;
        std::ifstream readStream("lastResult.txt");
        if (readStream.good()) {
            string text;
            readStream >> text;
            lastResult = std::stoll(text);
        }
        long long sum = 0;
        for (long long time : times) sum += time;
        long long average = sum / (long long)times.size();
        bool noChange = std::abs(average - lastResult) <= ticksNearby;
        string change = average > lastResult ? colorGreen : colorRed;
        char changeChar = average > lastResult ? '+' : '-';
        if (noChange) change = colorYellow;
        std::cout << "Average Ticks p/s: " << change << changeChar << "[" << average << "] "
        << colorReset << "(" << lastResult << ")" << std::endl;
        if (!noChange) {
            std::ofstream writeStream("lastResult.txt");
            writeStream << average;
        }
    }

    void Display::close() {
        glDeleteTextures(1, &oamTexture);
        glDeleteTextures(1, &paletteTexture);
        glDeleteTextures(1, &nameTableTexture);
        glDeleteTextures(1, &patternTexture);

        glDeleteSamplers(1, &sampler);
        glDeleteVertexArrays(1, &vao);

        glDeleteProgram(spriteProgram);
        glDeleteProgram(backgroundProgram);

        calcPerformance();
    }

    void Display::nextTick(long long nTick) { currentTick = nTick; }

    Display::Display(Clock* nClock, PPU* nPpu, string title) : clock(nClock), ppu(nPpu) {
        if (!glfwInit()) throw CouldNotCreateWindowException();

        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

        window = glfwCreateWindow(
                (int)(256 * windowSizeMultiplier),
                (int)(240 * windowSizeMultiplier),
                title.c_str(),
                nullptr, nullptr);
        if (!window) throw CouldNotCreateWindowException();

        glfwSetWindowUserPointer(window, this);
        glfwSetKeyCallback(window, windowKeyInput);

        glfwMakeContextCurrent(window);
        glfwSwapInterval(0);

        times.reserve(1000);

        init();
    }

    Display::~Display() {
        close();
        glfwDestroyWindow(window);
        glfwTerminate();
    }
}