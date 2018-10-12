//
// Created by Taylor Whatley on 2018-10-06.
//

#ifndef NEM_DISPLAY_H
#define NEM_DISPLAY_H

#include "../Controller/Controller.h"
#include "../Emulator.h"

#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>

#include <thread>

namespace Nem {
    class Display {
        // GLFW
        GLFWwindow* window;

        // Nemulator
        Emulator emulator;
        EditController mainController;
        NullController nullController;
        std::thread* cpuThread = nullptr;

        // OpenGL
        GLuint backgroundProgram, spriteProgram;
        GLuint vao;
        GLuint sampler;
        GLuint patternTexture, nameTableTexture, paletteTexture, oamTexture;
        GLint uniformBkgPatternSampler, uniformBkgNameTableSampler, uniformBkgPaletteSampler;
        GLint uniformBkgNameTableDrawIndex, uniformBkgPatternTableDrawIndex;
        GLint uniformBkgScrollX, uniformBkgScrollY;
        GLint uniformSprPatternSampler, uniformSprPaletteSampler, uniformSprOAMSampler;
        GLint uniformSprPatternTableDrawIndex;

        vector<GLuint> makePatternData(PPU *ppu);
        vector<GLuint> makeNameTableData(PPU* ppu);
        vector<GLuint> makeOAMData(PPU* ppu);

        void forceShow();
        bool loadShaders();
        void loadUniforms();

        void refreshPatternTable();
        void refreshNameTable();
        void refreshPaletteRam();
        void refreshOAM();
        void checkEdits();

        bool init();
        void render();
        void loop();
    public:
        void keyInput(int key, int action);

        void exec();

        explicit Display(string pathToRom);
        ~Display();
    };
}

#endif //NEM_DISPLAY_H
