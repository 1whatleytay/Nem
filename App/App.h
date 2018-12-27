//
// Created by Taylor Whatley on 2018-10-06.
//

#ifndef NEM_APP_H
#define NEM_APP_H

#include "../Controller/Controller.h"
#include "../Emulator.h"

#include "../Util/Stopwatch.h"

#include <thread>
#include <unordered_map>

#ifdef _WIN32
#include <GL/gl3w.h>
#define GLFW_INCLUDE_NONE
#else
#define GLFW_INCLUDE_GLCOREARB
#endif

#include <GLFW/glfw3.h>

struct GLFWwindow;

namespace Nem {
    void checkDebugBinding(string name);

    struct Vertex {
        GLfloat x, y;
        GLfloat texX, texY;
        GLint id;
    };

    class Display {
        PPU* ppu = nullptr;
        GLFWwindow* window = nullptr;

        GLuint program;
        GLuint vao;
        GLuint sampler;
        GLuint palette, patternTable[2];
        GLuint nameTables[2];

        GLint uniPaletteRam[4];

        constexpr static int nesWidth = 256, nesHeight = 240;
        constexpr static float windowAmp = 2.0f;
        constexpr static int windowWidth = (int)(nesWidth * windowAmp);
        constexpr static int windowHeight = (int)(nesHeight * windowAmp);

        Stopwatch stopwatch;
        vector<int> times;
        void calculateTimes();

        long long processedTick = 0;
        void skipCycles(long long cycles);

        void loadShaders();
        void checkEdits();

        bool init();
        void close();
    public:
        EditController mainController;
        EditController secondController;
        std::unordered_map<int, Buttons> mainControllerBindings;
        std::unordered_map<int, Buttons> secondControllerBindings;

        void pressKey(int key, int action);
        void exec();

        explicit Display(PPU* ppu);
        ~Display();

        void checkGL(string test);
    };

    class App : private Emulator {
        Display display;

        std::thread* cpuThread = nullptr;

        void loadBindings();
    public:
        void exec();

        explicit App(string pathToRom);
        ~App();
    };
}

#endif //NEM_APP_H
