//
// Created by Taylor Whatley on 2018-10-06.
//

#ifndef NEM_DISPLAY_H
#define NEM_DISPLAY_H

#include "../Controller/Controller.h"
#include "../Emulator.h"

#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>

#include <OpenAL/al.h>
#include <OpenAL/alc.h>

#include <thread>
#include <unordered_map>

namespace Nem {
    class Audio {
        APU* apu;

        ALCdevice* device = nullptr;
        ALCcontext* context = nullptr;

        ALuint testBuffer;
        GLuint source;

        volatile bool stopExecution = false;

        bool init();
        void loop();
        void close();
    public:
        void exec();
        void stopExec();

        explicit Audio(APU* nApu);
        ~Audio();
    };

    class Display {
        PPU* ppu;

        // Interface
        GLFWwindow* window;

        // OpenGL
        GLuint backgroundProgram, spriteProgram;
        GLuint vao;
        GLuint sampler;
        GLuint patternTexture, nameTableTexture, paletteTexture, oamTexture;
        GLint uniformBkgGrayscale, uniformSprGrayscale;
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
        void refreshRegisters();
        void checkEdits();

        bool init();
        void render();
        void loop();
        void close();
    public:
        EditController mainController;
        EditController secondController;
        std::unordered_map<int, Buttons> mainControllerBindings;
        std::unordered_map<int, Buttons> secondControllerBindings;

        void keyInput(int key, int action);

        void exec();

        explicit Display(PPU* nPpu, string title = "Nemulator");
        ~Display();
    };

    class App : private Emulator {
        Display* display = nullptr;
        Audio* audio = nullptr;

        std::thread* cpuThread = nullptr;
        std::thread* ppuThread = nullptr;
        std::thread* audioThread = nullptr;

        void loadBindings();
    public:
        void exec();

        App(string pathToRom);
        ~App();
    };
}

#endif //NEM_DISPLAY_H
