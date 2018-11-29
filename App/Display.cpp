//
// Created by Taylor Whatley on 2018-10-06.
//

#include "App.h"

#include "../CPU/CPU.h"
#include "../PPU/PPU.h"
#include "../Util/Clock.h"

#include <GLFW/glfw3.h>

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
        bool ppuNeedsRefresh = ppu->memory->checkNeedsRefresh();
        if (ppu->memory->edits.paletteRam) refreshPaletteRam();
        if (ppu->memory->edits.nameTable) refreshNameTable();
        if (ppu->memory->edits.oam) refreshOAM();
        if (ppu->memory->edits.registers) refreshRegisters();
        if (ppu->memory->edits.patternTable || ppuNeedsRefresh) refreshPatternTable();

        ppu->memory->edits.reset();
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
                     0, GL_RED_INTEGER, GL_UNSIGNED_INT, nullptr);
        glBindSampler(0, sampler);
        refreshPatternTable();

        glGenTextures(1, &nameTableTexture);
        glActiveTexture(GL_TEXTURE0 + 1);
        glBindTexture(GL_TEXTURE_1D, nameTableTexture);
        glTexImage1D(GL_TEXTURE_1D, 0, GL_R32UI, kilobyte(2),
                     0, GL_RED_INTEGER, GL_UNSIGNED_INT, nullptr);
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

        stopwatch.start();

        return true;
    }

//    void Display::render() {
//        glClear(GL_COLOR_BUFFER_BIT);
//
//        GLenum error = glGetError();
//        if (error != GL_NO_ERROR) std::cout << "OpenGL Error: " << error << std::endl;
//
//        if (ppu->isControlSet(PPURegisters::ControlFlags::SprSize)) {
//            std::cout << "Sprite Size 8x16 is unsupported." << std::endl;
//        }
//
//        if (ppu->isMaskSet(PPURegisters::MaskFlags::ShowBKG)) {
//            glUseProgram(backgroundProgram);
//
//            glUniform1i(uniformBkgPatternTableDrawIndex,
//                        ppu->isControlSet(PPURegisters::ControlFlags::BkgPatternTable));
//
//            int val = 0;
//            glUniform1i(uniformBkgScrollX, val);
//            glUniform1i(uniformBkgNameTableDrawIndex, 0);
//            glDrawArrays(GL_TRIANGLES, 0, 30 * 32 * 6);
//        }
//
//        if (ppu->isMaskSet(PPURegisters::MaskFlags::ShowSPR)) {
//            glUseProgram(spriteProgram);
//
//            glUniform1i(uniformSprPatternTableDrawIndex,
//                        ppu->isControlSet(PPURegisters::ControlFlags::SprPatternTable));
//
//            glDrawArrays(GL_TRIANGLES, 0, 64 * 6);
//        }
//    }

    void Display::keyInput(int key, int action) {
        if (key == GLFW_KEY_K || action == GLFW_PRESS)
            setDebugFlag("profiler-execution-analysis", !getDebugFlag("profiler-execution-analysis"));
        if (mainControllerBindings.find(key) != mainControllerBindings.end()) {
            if (action == GLFW_PRESS) mainController.press(mainControllerBindings[key]);
            else if (action == GLFW_RELEASE) mainController.release(mainControllerBindings[key]);
        } else if (secondControllerBindings.find(key) != secondControllerBindings.end()) {
            if (action == GLFW_PRESS) secondController.press(secondControllerBindings[key]);
            else if (action == GLFW_RELEASE) secondController.release(secondControllerBindings[key]);
        }
    }

//    void Display::loop() {
//        ppu->start();
//        while (!glfwWindowShouldClose(window)) {
//            glfwPollEvents();
//
//            ppu->registers->status |= PPURegisters::StatusFlags::SprZeroHit;
//
//            checkEdits();
//            render();
//            glfwSwapBuffers(window);
//
//            ppu->waitCycles(240 * 341);
//            ppu->registers->status &= ~PPURegisters::StatusFlags::SprZeroHit;
//
//            if (ppu->isControlSet(PPURegisters::ControlFlags::VBlankNMI)) ppu->postNMI();
//            ppu->registers->status |= PPURegisters::StatusFlags::VBlank;
//            ppu->waitCycles(22 * 341);
//            ppu->registers->status &= ~PPURegisters::StatusFlags::VBlank;
//
//            ppu->oddFrame = !ppu->oddFrame;
//        }
//    }

    void Display::close() {
        glDeleteTextures(1, &oamTexture);
        glDeleteTextures(1, &paletteTexture);
        glDeleteTextures(1, &nameTableTexture);
        glDeleteTextures(1, &patternTexture);

        glDeleteSamplers(1, &sampler);
        glDeleteVertexArrays(1, &vao);

        glDeleteProgram(spriteProgram);
        glDeleteProgram(backgroundProgram);
    }

//    void Display::exec() {
//        if (init()) loop();
//        close();
//    }
    long long lastFrame = 0;

    void Display::nextTick(long long cTick) {
        if (cTick == -1) return;
        long long tick = cTick - lastScanline;

        if (scanlineId == 0 && tick == 0) {
            std::cout << "Lapping and polling." << std::endl;
            if (stopwatch.hasBeen(1000)) {
                stopwatch.stop();
                std::cout << "FPS: " << frame - lastFrame << std::endl;
                lastFrame = frame;
                stopwatch.start();
            }
            glfwPollEvents();
            if (glfwWindowShouldClose(window)) clock->stopExec();
            if (ppu->isMaskSet(PPURegisters::MaskFlags::ShowBKG) && frame % 2 == 1) tick++;
        }

        if (scanlineId >= 0 && scanlineId <= 239) {
            checkEdits();
            if (ppu->isMaskSet(PPURegisters::MaskFlags::ShowBKG) && scanlineId % 8 == 0) {
                glUseProgram(backgroundProgram);
                glUniform1i(uniformBkgPatternTableDrawIndex,
                    ppu->isControlSet(PPURegisters::ControlFlags::BkgPatternTable));
                glDrawArrays(GL_TRIANGLES, (GLint)scanlineId / 8 * 32 * 6, 30 * 32 * 6);
            }
            if (ppu->isMaskSet(PPURegisters::MaskFlags::ShowSPR) && scanlineId == 239 && tick == 340) {
                glUseProgram(spriteProgram);
                glUniform1i(uniformSprPatternTableDrawIndex,
                    ppu->isControlSet(PPURegisters::ControlFlags::SprPatternTable));
                glDrawArrays(GL_TRIANGLES, 0, 64 * 6);
            }
        }
        if (scanlineId >= 240 && scanlineId <= 261) {
            if (scanlineId == 241 && tick == 1) {
                ppu->registers->status |= PPURegisters::StatusFlags::VBlank;
                if (ppu->isControlSet(PPURegisters::ControlFlags::VBlankNMI)) ppu->postNMI();
                glfwSwapBuffers(window);
                glClear(GL_COLOR_BUFFER_BIT);
            }
            if (scanlineId == 261) {
                ppu->registers->status &= ~PPURegisters::StatusFlags::SprOverflow;
                ppu->registers->status &= ~PPURegisters::StatusFlags::VBlank;
                ppu->registers->status &= ~PPURegisters::StatusFlags::SprZeroHit;
            }
        }
        if (tick >= 340) {
            if (scanlineId >= 261) {
                frame++;
            }
            lastScanline = cTick + 1;
            scanlineId++;
            scanlineId = scanlineId % 262;
        }
    }

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

        init();
    }

    Display::~Display() {
        close();
        glfwDestroyWindow(window);
        glfwTerminate();
    }
}