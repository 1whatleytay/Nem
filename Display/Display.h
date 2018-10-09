//
// Created by Taylor Whatley on 2018-10-06.
//

#ifndef NEM_DISPLAY_H
#define NEM_DISPLAY_H

#include "../Nem.h"

#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>

#include <thread>

namespace Nem {
    class Display {
        // GLFW
        GLFWwindow* window;

        // Nemulator
        Emulator emulator;
        std::thread* cpuThread = nullptr;

        // OpenGL
        GLuint backgroundProgram, spriteProgram;
        GLuint vao;
        GLuint sampler;
        GLuint patternTexture, nameTableTexture, paletteTexture;
        GLint uniformBkgPatternSampler, uniformBkgNameTableSampler, uniformBkgPaletteSampler;
        GLint uniformBkgNameTableDrawIndex, uniformBkgPatternTableDrawIndex;

        vector<GLuint> makePatternData(PPU *ppu);
        vector<GLuint> makeNameTableData(PPU* ppu);

        void forceShow();
        bool loadShaders();
        void loadUniforms();
        void loadPPUPalette(GLuint program, Nem::PPUPalette palette, string location);

        void refreshPatternTable();
        void refreshNameTable();
        void refreshPaletteRam();
        void checkEdits();

        bool init();
        void render();
        void loop();
    public:
        void exec();

        explicit Display(string pathToRom);
        ~Display();
    };
}

#endif //NEM_DISPLAY_H
