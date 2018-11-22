//
// Created by Taylor Whatley on 2018-10-06.
//

#ifndef NEM_DISPLAY_H
#define NEM_DISPLAY_H

#include "../Controller/Controller.h"
#include "../Emulator.h"

#ifndef CPU_ONLY
#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>

#ifndef NO_AUDIO
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#endif
#endif

#include <thread>
#include <unordered_map>

namespace Nem {
#ifndef CPU_ONLY
#ifndef NO_AUDIO
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
#endif

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
#endif

    class App : private Emulator {
#ifndef CPU_ONLY
        Display* display = nullptr;
#ifndef NO_AUDIO
        Audio* audio = nullptr;
#endif
#endif

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
