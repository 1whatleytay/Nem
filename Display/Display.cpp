//
// Created by Taylor Whatley on 2018-10-06.
//

#include "Display.h"

#include "../CPU/CPU.h"
#include "../PPU/PPU.h"
#include "../ROM/ROM.h"
#include "../Errors.h"

#include <GLFW/glfw3.h>

#include <iostream>

namespace Nem {
    const double windowSizeMultiplier = 2.5;
    const int spriteCount = 0x2000 / 16;

    void windowKeyInput(GLFWwindow* window, int key, int scancode, int action, int mods) {
        Display* display = (Display*)glfwGetWindowUserPointer(window);
        display->keyInput(key, action);
    }

    void loadPPUPalette(GLuint program, Nem::PPUPalette palette, string location) {
        GLint colorA = glGetUniformLocation(program, (location + ".colorA").c_str());
        GLint colorB = glGetUniformLocation(program, (location + ".colorB").c_str());
        GLint colorC = glGetUniformLocation(program, (location + ".colorC").c_str());

        glUseProgram(program);
        glUniform1ui(colorA, (GLuint)palette.colorA);
        glUniform1ui(colorB, (GLuint)palette.colorB);
        glUniform1ui(colorC, (GLuint)palette.colorC);
    }

    void Display::forceShow() {
        // Workaround for macOS mojave's window update glitch
        int x, y;
        glfwGetWindowPos(window, &x, &y);
        glfwPollEvents();
        glfwSwapBuffers(window);
        glfwSetWindowPos(window, x + 1, y);
    }

    void Display::refreshPatternTable() {
        vector<GLuint> patternTableTex = makePatternData(emulator.ppu);

//        std::cout << "Sprite Count: " << spriteCount << std::endl;
//        std::cout << "[" << std::endl;
//        for (int a = 0; a < spriteCount; a++) {
//            std::cout << "\t[" << a << std::endl;
//            for (int y = 0; y < 8; y++) {
//                std::cout << "\t\t";
//                for (int x = 0; x < 8; x++) {
//                    std::cout << patternTableTex[a * 8 * 8 + y * 8 + x];
//                }
//                std::cout << std::endl;
//            }
//            std::cout << "\t]" << std::endl;
//        }
//        std::cout << "]" << std::endl;

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

        loadPPUPalette(backgroundProgram, emulator.ppu->memory->palettes.background[0], "palettes[0]");
        loadPPUPalette(backgroundProgram, emulator.ppu->memory->palettes.background[1], "palettes[1]");
        loadPPUPalette(backgroundProgram, emulator.ppu->memory->palettes.background[2], "palettes[2]");
        loadPPUPalette(backgroundProgram, emulator.ppu->memory->palettes.background[3], "palettes[3]");

        loadPPUPalette(spriteProgram, emulator.ppu->memory->palettes.sprite[0], "palettes[0]");
        loadPPUPalette(spriteProgram, emulator.ppu->memory->palettes.sprite[1], "palettes[0]");
        loadPPUPalette(spriteProgram, emulator.ppu->memory->palettes.sprite[2], "palettes[0]");
        loadPPUPalette(spriteProgram, emulator.ppu->memory->palettes.sprite[3], "palettes[0]");
    }

    void Display::refreshOAM() {
        vector<GLuint> oamTex = makeOAMData(emulator.ppu);

//        for (int a = 0; a < 64; a++) {
//            std::cout << "SPRITE: " << a << std::endl;
//            std::cout << "\t x: " << oamTex[a * 4 + 3] << " y: " << oamTex[a * 4 + 0] << std::endl;
//            std::cout << "\t pattern: " << oamTex[a * 4 + 1] << " flags: " << oamTex[a * 4 + 2] << std::endl;
//        }

        //if (oamTex[0] > 240) emulator.ppu->registers->status |= 0b01000000;
        //else emulator.ppu->registers->status &= 0b01000000;

        glActiveTexture(GL_TEXTURE0 + 3);
        glBindTexture(GL_TEXTURE_1D, oamTexture);
        glTexSubImage1D(GL_TEXTURE_1D,
                0, 0, 64 * 4,
                GL_RED_INTEGER, GL_UNSIGNED_INT, &oamTex[0]);
    }

    void Display::checkEdits() {
        if (emulator.ppu->memory->edits.paletteRam) refreshPaletteRam();
        if (emulator.ppu->memory->edits.patternTable) refreshPatternTable();
        if (emulator.ppu->memory->edits.nameTable) refreshNameTable();
        if (emulator.ppu->memory->edits.oam) refreshOAM();

        emulator.ppu->memory->edits.reset();
    }

    bool Display::init() {
#ifdef MARIO_8057
        emulator.cpu->wait8057();
#endif

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
        return true;
    }

    void Display::render() {
        glClear(GL_COLOR_BUFFER_BIT);

        if (emulator.ppu->isControlSet(PPURegisters::ControlFlags::SprSize)) {
            std::cout << "Sprite Size 8x16 is unsupported." << std::endl;
        }

        if (emulator.ppu->isMaskSet(PPURegisters::MaskFlags::ShowBKG)) {
            glUseProgram(backgroundProgram);

            glUniform1i(uniformBkgPatternTableDrawIndex,
                    emulator.ppu->isControlSet(PPURegisters::ControlFlags::BkgPatternTable));
            glUniform1i(uniformBkgNameTableDrawIndex, 0);

            glDrawArrays(GL_TRIANGLES, 0, 32 * 30 * 6); // Draw Background
        }

        if (emulator.ppu->isMaskSet(PPURegisters::MaskFlags::ShowSPR)) {
            glUseProgram(spriteProgram);

            glUniform1i(uniformSprPatternTableDrawIndex,
                        emulator.ppu->isControlSet(PPURegisters::ControlFlags::SprPatternTable));

            glDrawArrays(GL_TRIANGLES, 0, 64 * 6);
        }
    }

    void Display::keyInput(int key, int action) {
        switch (key) {
            case GLFW_KEY_X:
                if (action == GLFW_PRESS) mainController.press(Buttons::ButtonA);
                if (action == GLFW_RELEASE) mainController.release(Buttons::ButtonA);
                break;
            case GLFW_KEY_Z:
                if (action == GLFW_PRESS) mainController.press(Buttons::ButtonB);
                if (action == GLFW_RELEASE) mainController.release(Buttons::ButtonB);
                break;
            case GLFW_KEY_UP:
                if (action == GLFW_PRESS) mainController.press(Buttons::ButtonUp);
                if (action == GLFW_RELEASE) mainController.release(Buttons::ButtonUp);
                break;
            case GLFW_KEY_DOWN:
                if (action == GLFW_PRESS) mainController.press(Buttons::ButtonDown);
                if (action == GLFW_RELEASE) mainController.release(Buttons::ButtonDown);
                break;
            case GLFW_KEY_LEFT:
                if (action == GLFW_PRESS) mainController.press(Buttons::ButtonLeft);
                if (action == GLFW_RELEASE) mainController.release(Buttons::ButtonLeft);
                break;
            case GLFW_KEY_RIGHT:
                if (action == GLFW_PRESS) mainController.press(Buttons::ButtonRight);
                if (action == GLFW_RELEASE) mainController.release(Buttons::ButtonRight);
                break;
            case GLFW_KEY_ENTER:
                if (action == GLFW_PRESS) mainController.press(Buttons::ButtonStart);
                if (action == GLFW_RELEASE) mainController.release(Buttons::ButtonStart);
                break;
            case GLFW_KEY_SPACE:
                if (action == GLFW_PRESS) mainController.press(Buttons::ButtonSelect);
                if (action == GLFW_RELEASE) mainController.release(Buttons::ButtonSelect);
                break;
            default: break;
        }
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
        std::cout << emulator.rom->getRomInfo() << std::endl;

        cpuThread = new std::thread(&CPU::exec, emulator.cpu);

        if (!glfwInit()) throw CouldNotCreateWindowException();

        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

        window = glfwCreateWindow(
                (int)(256 * windowSizeMultiplier),
                (int)(240 * windowSizeMultiplier),
                ("Nemulator - " + emulator.rom->getRomName()).c_str(),
                nullptr, nullptr);
        if (!window) throw CouldNotCreateWindowException();

        emulator.setController(0, &mainController);
        emulator.setController(1, &nullController);

        glfwSetWindowUserPointer(window, this);
        glfwSetKeyCallback(window, windowKeyInput);

        glfwMakeContextCurrent(window);
        glfwSwapInterval(0);
    }

    Display::~Display() {
        emulator.cpu->stopExec();
        if (cpuThread) cpuThread->join();
        delete cpuThread;

        glfwDestroyWindow(window);
        glfwTerminate();
    }
}