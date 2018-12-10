//
// Created by Taylor Whatley on 2018-10-06.
//

#ifndef NEM_APP_H
#define NEM_APP_H

#include "../Controller/Controller.h"
#include "../Emulator.h"

#include "../Util/Stopwatch.h"

#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>

#ifndef NO_AUDIO
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#endif

#include <thread>
#include <unordered_map>

namespace Nem {
    class Audio {
        APU* apu;

#ifndef NO_AUDIO
        ALCdevice* device = nullptr;
        ALCcontext* context = nullptr;

        ALuint testBuffer;
        ALuint source;

        volatile bool stopExecution = false;

        bool init();
        void loop();
        void close();
    public:
        void exec();
        void stopExec();

        explicit Audio(APU* nApu);
        ~Audio();
#endif
    };

    class Display {
        PPU* ppu = nullptr;
        Clock* clock = nullptr;

        // Interface
        GLFWwindow* window = nullptr;

        // Debug and Performance
        NanoStopwatch stop;
        vector<long long> times;

        void calcPerformance();

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

        volatile bool stopExecution = false;

        volatile long long currentTick = 0, processedTick = 0;
        long long frame = 0;

        inline void skipCycles(long long num);

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
        void close();
    public:
        EditController mainController;
        EditController secondController;
        std::unordered_map<int, Buttons> mainControllerBindings;
        std::unordered_map<int, Buttons> secondControllerBindings;

        void exec();
        void keyInput(int key, int action);
        void nextTick(long long nTick);

        explicit Display(Clock* nClock, PPU* nPpu, string title = "Nemulator");
        ~Display();
    };

    class App : private Emulator {
        Display* display = nullptr;
        Audio* audio = nullptr;

        std::thread* cpuThread = nullptr;
        std::thread* clockThread = nullptr;
        std::thread* audioThread = nullptr;

        void loadBindings();
    public:
        void exec();

        explicit App(string pathToRom);
        ~App();
    };
}

#endif //NEM_APP_H
