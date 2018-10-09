//
// Created by Taylor Whatley on 2018-09-28.
//

#include "QNemWindow.h"

#include "../Errors.h"

#include <iostream>

#ifndef TIMER_TYPE
#define TIMER_TYPE Qt::CoarseTimer
#endif

GLuint* makePatternData(Nem::ROM* rom) {
    GLuint* data = new GLuint[(rom->chrROM.size() / 16) * 8 * 8];
    for (int a = 0; a < rom->chrROM.size() / 16; a++) {
        for (int b = 0; b < 8; b++) {
            Byte bit = 0b1000000;
            for (int c = 0; c < 8; c++) {
                data[(a * 8 * 8) + b * 8 + c] =
                        (GLuint)(
                                (((rom->chrROM[a * 16 + 8 + b] & bit) == bit) << 1) +
                                ((rom->chrROM[a * 16 + b] & bit) == bit));
                bit = bit >> 1;
            }
        }
    }
    return data;
}

GLuint* makeNameTableData(Nem::PPU* ppu) {
    GLuint* data = new GLuint[kilobyte(2)];
    for (int a = 0; a < kilobyte(1); a++) {
        data[a] = ppu->memory->nameTables[0][a];
        data[a + kilobyte(1)] = ppu->memory->nameTables[1][a];
    }
    return data;
}

void QNemWindow::checkOpenGLErrors(string subtitle) {
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) std::cout << "OpenGL Error " << subtitle << ": " << error << std::endl;
}

void QNemWindow::init() {
#ifdef OPENGL_DEFAULT
    QSurfaceFormat format = requestedFormat();
#else
    QSurfaceFormat format;
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
#endif

    context = new QOpenGLContext(this);
    context->setFormat(format);
    context->create();
    context->makeCurrent(this);

    initializeOpenGLFunctions();

#ifdef MARIO_8057
    emulator.cpu->wait8057();
#endif

    if (!loadShaders()) exit(0);
    loadUniforms();

    emulator.ppu->memory->palettes.setClearColor(0x0d);
    Nem::Color clearColor = Nem::palette2C02[emulator.ppu->memory->palettes.getClearColor()];
    glClearColor(clearColor.red, clearColor.green, clearColor.blue, 1);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenSamplers(1, &sampler);
    glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_R, GL_REPEAT);

    std::cout << "Loading... " << std::endl;

    int spriteCount = (int)(emulator.rom->chrROM.size() / 16);
    GLuint* patternTex = makePatternData(emulator.rom);

//    std::cout << "Sprite Count: " << spriteCount << std::endl;
//    std::cout << "[" << std::endl;
//    for (int a = 0; a < spriteCount; a++) {
//        std::cout << "\t[" << std::endl;
//        for (int y = 0; y < 8; y++) {
//            std::cout << "\t\t";
//            for (int x = 0; x < 8; x++) {
//                std::cout << patternTex[a * 8 * 8 + y * 8 + x];
//            }
//            std::cout << std::endl;
//        }
//        std::cout << "\t]" << std::endl;
//    }
//    std::cout << "]" << std::endl;

    glGenTextures(1, &patternTexture);
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_2D, patternTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, 8, spriteCount * 8, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, patternTex);
    glBindSampler(0, sampler);

    delete[] patternTex;

    // Hook up to real name table. (convert first!)
    //GLuint colors[kilobyte(2)];
    //for (int a = 0; a < kilobyte(2); a++) colors[a] = (GLuint)(a * 2 % 0x40);

    GLuint* nameTableTex = makeNameTableData(emulator.ppu);

    glGenTextures(1, &nameTableTexture);
    glActiveTexture(GL_TEXTURE0 + 1);
    glBindTexture(GL_TEXTURE_1D, nameTableTexture);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_R32UI, kilobyte(2), 0, GL_RED_INTEGER, GL_UNSIGNED_INT, nameTableTex);
    glBindSampler(1, sampler);

    delete[] nameTableTex;

    initialized = true;
}

void QNemWindow::clearColorListener(Byte value) {
    if (initialized) {

    }
}

void QNemWindow::backgroundListener(int index, Nem::PPUPalette palette) {
    if (initialized) {

    }
}

void QNemWindow::spriteListener(int index, Nem::PPUPalette palette) {
    if (initialized) {

    }
}

void QNemWindow::render() {
    if (!isExposed()) return;
    if (!initialized) init();

    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(backgroundProgram);
    glUniform1i(uniformBkgNameTableDrawIndex, 0); // Set Table to 0
    glUniform1i(uniformBkgPatternTableDrawIndex, 0); // Set Pattern Table to 0
    glDrawArrays(GL_TRIANGLES, 0, 30 * 32 * 6); // Draw Background

    checkOpenGLErrors("Main Loop");

    context->swapBuffers(this);

#ifndef GRAPHICS_ONLY
    emulator.ppu->vblank();
#endif
}

bool QNemWindow::event(QEvent* event) {
    switch(event->type()) {
        case QEvent::UpdateRequest:
            render();
            return true;
        default:
            return QWindow::event(event);
    }
}

void QNemWindow::timerEvent(QTimerEvent* event) {
    requestUpdate();
}

QNemWindow::QNemWindow(string romPath) : emulator(romPath) {
#ifndef GRAPHICS_ONLY
    cpuThread = new std::thread(&Nem::CPU::exec, emulator.cpu);
#endif
    emulator.ppu->memory->palettes.clearColorListener =
        std::bind(&QNemWindow::clearColorListener, this, std::placeholders::_1);
    emulator.ppu->memory->palettes.backgroundListener =
            std::bind(&QNemWindow::backgroundListener, this, std::placeholders::_1, std::placeholders::_2);
    emulator.ppu->memory->palettes.spriteListener =
            std::bind(&QNemWindow::spriteListener, this, std::placeholders::_1, std::placeholders::_2);

    setTitle("Nemulator");
    setBaseSize(QSize(256, 240));
    setSurfaceType(OpenGLSurface);

    timer = startTimer(1000/60);

    show();
}

QNemWindow::~QNemWindow() {
    killTimer(timer);

    emulator.cpu->stopExec();

    if (cpuThread) cpuThread->join();
    delete cpuThread;

    glDeleteSamplers(1, &sampler);
    glDeleteTextures(1, &nameTableTexture);
    glDeleteTextures(1, &patternTexture);
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(backgroundProgram);
    //glDeleteProgram(spriteProgram);

    delete context;
}