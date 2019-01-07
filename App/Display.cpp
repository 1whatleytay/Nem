//
// Created by Taylor on 2018-12-10.
//

#include "App.h"
#include "../PPU/PPU.h"

#include <iostream>

namespace Nem {
    void onError(int code, const char* description) {
        std::cout << "GLFW Error " << makeHex(code) << ": " << description << std::endl;
    }

    void onKeyPress(GLFWwindow *window, int key, int, int action, int) {
        Display* display = (Display *) glfwGetWindowUserPointer(window);
        display->pressKey(key, action);
    }

    void onResize(GLFWwindow*, int width, int height) {
        glViewport(0, 0, width, height);
    }

    void Display::pressKey(int key, int action) {
        if (mainControllerBindings.find(key) != mainControllerBindings.end()) {
            Byte buttons = (Byte) mainControllerBindings[key];
            if (action == GLFW_PRESS) mainController.press(buttons);
            else if (action == GLFW_RELEASE) mainController.release(buttons);
        }
        if (secondControllerBindings.find(key) != secondControllerBindings.end()) {
            Byte buttons = (Byte) secondControllerBindings[key];
            if (action == GLFW_PRESS) secondController.press(buttons);
            else if (action == GLFW_RELEASE) secondController.release(buttons);
        }
    }

    void Display::checkGL(string myTest) {
        GLenum error = glGetError();
        if (error != GL_NO_ERROR)
            std::cout << "OpenGL Error " << myTest << ": " << makeHex((int)error) << std::endl;
    }

    void Display::calculateTimes() {
        if (times.empty()) std::cout << "[Sample Empty]" << std::endl;
        int min = INT_MAX, max = INT_MIN;
        int sum = 0;
        for (int a : times) {
            sum += a;
            if (a < min) min = a;
            if (a > max) max = a;
        }
        int average = sum / (int)times.size();
        std::cout << "[Average Ticks: " << average << ", Min Ticks: " << min
        << ", Max Ticks: " << max << ", Size: " << times.size() << "]" << std::endl;
    }

    void Display::skipCycles(long long num) {
        stopwatch.lap += num;

        if (stopwatch.hasBeen(1000)) {
            times.push_back(stopwatch.lap);
            stopwatch.reset();
        }

        while (processedTick + num > ppu->ticks) { }
        processedTick += num;
    }

    void Display::exec() {
        if (init()) {
            stopwatch.start();
            while (!glfwWindowShouldClose(window)) {
                glfwPollEvents();
                checkEdits();

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_1D, palette);
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, patternTable[1]);

                glBindVertexArray(nameTableVAOs[0]);
                glBindBuffer(GL_ARRAY_BUFFER, nameTableBuffers[0]);

                for (int a = 0; a < 240; a++) {
                    if (a % 8 == 0) glDrawArrays(GL_TRIANGLES, (a / 8) * 32 * 6, 32 * 6);
                    skipCycles(261 - (a == 0 && ppu->isMaskSet(
                            PPURegisters::MaskFlags::ShowBKG) && ppu->oddFrame));
                }

                skipCycles(261 + 1);
                ppu->registers.status |= PPURegisters::StatusFlags::VBlank;
                if (ppu->isControlSet(PPURegisters::ControlFlags::VBlankNMI)) ppu->postNMI();

                checkGL("My Test");

                glfwSwapBuffers(window);
                glClear(GL_COLOR_BUFFER_BIT);

                skipCycles(260 + 261 * 19 + 1);

                ppu->registers.status &= ~PPURegisters::StatusFlags::SprOverflow;
                ppu->registers.status &= ~PPURegisters::StatusFlags::VBlank;
                ppu->registers.status &= ~PPURegisters::StatusFlags::SprZeroHit;

                skipCycles(260);

                ppu->oddFrame = !ppu->oddFrame;
            }
        }
        close();
    }

    bool Display::init() {
#ifdef _WIN32
        if (gl3wInit() != GL3W_OK) throw CouldNotCreateWindowException();
#endif
        times.reserve(1000);

        glDisable(GL_CULL_FACE);

        loadShaders();
        glUseProgram(program);

        glGenSamplers(1, &sampler);
        glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glGenTextures(1, &palette);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_1D, palette);
        glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, 0x40 * sizeof(GLfloat), 0,
                GL_RGB, GL_FLOAT, palette2C02);
        glBindSampler(0, sampler);

        glGenTextures(2, patternTable);
        glActiveTexture(GL_TEXTURE1);
        for (GLuint pat : patternTable) {
            glBindTexture(GL_TEXTURE_2D, pat);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_R8I, 8, 256 * 8, 0,
                    GL_RED_INTEGER, GL_UNSIGNED_BYTE, nullptr);
        }
        glBindSampler(1, sampler);

        glGenBuffers(2, nameTableBuffers);
        glGenVertexArrays(2, nameTableVAOs);
        for (int a = 0; a < 2; a++) {
            glBindVertexArray(nameTableVAOs[a]);
            glBindBuffer(GL_ARRAY_BUFFER, nameTableBuffers[a]);
            glBufferData(GL_ARRAY_BUFFER, 6 * 30 * 32 * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);

            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)(sizeof(GLfloat) * 0));
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)(sizeof(GLfloat) * 2));
            glVertexAttribIPointer(2, 1, GL_INT, sizeof(Vertex), (void *)(sizeof(GLfloat) * 4));
            glEnableVertexAttribArray(0);
            glEnableVertexAttribArray(1);
            glEnableVertexAttribArray(2);
        }

        ppu->memory.edits.fill();
        checkEdits();

        return true;
    }

    void Display::close() {
        calculateTimes();

        glDeleteTextures(2, patternTable);
        glDeleteTextures(1, &palette);

        glDeleteVertexArrays(2, nameTableVAOs);
        glDeleteBuffers(2, nameTableBuffers);

        glDeleteSamplers(1, &sampler);

        glDeleteProgram(program);
    }

    Display::Display(Nem::PPU *ppu) : ppu(ppu) {
        if (!glfwInit()) throw CouldNotCreateWindowException();

        glfwSetErrorCallback(onError);

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);

        window = glfwCreateWindow(windowWidth, windowHeight, "Nem", nullptr, nullptr);
        if (!window) throw CouldNotCreateWindowException();

        glfwSetWindowUserPointer(window, this);
        glfwSetKeyCallback(window, onKeyPress);
        glfwSetWindowSizeCallback(window, onResize);

        glfwMakeContextCurrent(window);
        glfwSwapInterval(0);
    }

    Display::~Display() {
        glfwDestroyWindow(window);

        glfwTerminate();
    }
}