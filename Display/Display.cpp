//
// Created by Taylor Whatley on 2018-10-06.
//

#include "Display.h"
#include "../Errors.h"

#include <GLFW/glfw3.h>

#include <iostream>

namespace Nem {
    const double windowSizeMultiplier = 2.5;
    const int spriteCount = 0x2000 / 16;

    void Display::forceShow() {
        // Workaround for macOS mojave's window update glitch
        int x, y;
        glfwGetWindowPos(window, &x, &y);
        glfwPollEvents();
        glfwSwapBuffers(window);
        glfwSetWindowPos(window, x + 1, y);
    }

    bool Display::init() {
#ifdef MARIO_8057
        emulator.cpu->wait8057();
#endif

        if (!loadShaders()) return false;
        loadUniforms();

        glClearColor(1, 1, 0, 1);

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glGenSamplers(1, &sampler);
        glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glSamplerParameteri(sampler, GL_TEXTURE_WRAP_R, GL_REPEAT);

        vector<GLuint> patternTex = makePatternData(emulator.ppu);

        glGenTextures(1, &patternTexture);
        glActiveTexture(GL_TEXTURE0 + 0);
        glBindTexture(GL_TEXTURE_2D, patternTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, 8, spriteCount * 8,
                0, GL_RED_INTEGER, GL_UNSIGNED_INT, &patternTex[0]);
        glBindSampler(0, sampler);

        vector<GLuint> nameTableTex = makeNameTableData(emulator.ppu);

        glGenTextures(1, &nameTableTexture);
        glActiveTexture(GL_TEXTURE0 + 1);
        glBindTexture(GL_TEXTURE_1D, nameTableTexture);
        glTexImage1D(GL_TEXTURE_1D, 0, GL_R32UI, kilobyte(2),
                0, GL_RED_INTEGER, GL_UNSIGNED_INT, &nameTableTex[0]);
        glBindSampler(1, sampler);

        glGenTextures(1, &paletteTexture);
        glActiveTexture(GL_TEXTURE0 + 2);
        glBindTexture(GL_TEXTURE_1D, paletteTexture);
        glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB8, 0x40, 0, GL_RGB, GL_FLOAT, palette2C02);
        glBindSampler(2, sampler);

        refreshPaletteRam();

        forceShow();
        return true;
    }

    void Display::refreshPatternTable() {
        vector<GLuint> patternTableTex = makeNameTableData(emulator.ppu);

        glActiveTexture(GL_TEXTURE0 + 0);
        glBindTexture(GL_TEXTURE_2D, patternTexture);
        glTexSubImage2D(GL_TEXTURE_2D,
                        0, 0, 0, 8, spriteCount * 8,
                        GL_RED_INTEGER, GL_UNSIGNED_INT, &patternTableTex[0]);
    }
    void Display::refreshNameTable() {
        vector<GLuint> nameTableTex = makeNameTableData(emulator.ppu);

        glActiveTexture(GL_TEXTURE0 + 1);
        glBindTexture(GL_TEXTURE_1D, nameTableTexture);
        glTexSubImage1D(GL_TEXTURE_1D,
                        0, 0, kilobyte(2),
                        GL_RED_INTEGER, GL_UNSIGNED_INT, &nameTableTex[0]);
    }
    void Display::refreshPaletteRam() {
        Color clearColor = palette2C02[emulator.ppu->memory->getByte(0x3f00)];
        glClearColor(clearColor.red, clearColor.green, clearColor.blue, 1);

        // Background Palettes
        loadPPUPalette(backgroundProgram, emulator.ppu->memory->palettes.background[0], "palettes[0]");
        loadPPUPalette(backgroundProgram, emulator.ppu->memory->palettes.background[1], "palettes[1]");
        loadPPUPalette(backgroundProgram, emulator.ppu->memory->palettes.background[2], "palettes[2]");
        loadPPUPalette(backgroundProgram, emulator.ppu->memory->palettes.background[3], "palettes[3]");
    }

    void Display::checkEdits() {
        if (emulator.ppu->memory->edits.paletteRam) refreshPaletteRam();
        if (emulator.ppu->memory->edits.patternTable) refreshPatternTable();
        if (emulator.ppu->memory->edits.nameTable) refreshNameTable();

        emulator.ppu->memory->edits.reset();
    }

    void Display::render() {
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(backgroundProgram);
        glUniform1i(uniformBkgPatternTableDrawIndex, 1);
        glDrawArrays(GL_TRIANGLES, 0, 32 * 30 * 6); // Draw Background
    }

    void Display::loop() {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();

            checkEdits();

            render();

            glfwSwapBuffers(window);

            emulator.ppu->waitUntilNextFrame();
            emulator.ppu->nextFrame();
        }
    }

    void Display::exec() {
        if (init()) loop();
    }

    Display::Display(string pathToRom) : emulator(pathToRom) {
        cpuThread = new std::thread(&CPU::exec, emulator.cpu);

        if (!glfwInit()) throw CouldNotCreateWindowException();

        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

        window = glfwCreateWindow(
                (int)(256 * windowSizeMultiplier),
                (int)(240 * windowSizeMultiplier),
                "Nemulator",
                nullptr, nullptr);
        if (!window) throw CouldNotCreateWindowException();

        glfwMakeContextCurrent(window);
        glfwSwapInterval(0);
    }

    Display::~Display() {
        glfwDestroyWindow(window);
        glfwTerminate();

        emulator.cpu->stopExec();
        if (cpuThread) cpuThread->join();
        delete cpuThread;
    }
}