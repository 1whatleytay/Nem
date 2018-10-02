//
// Created by Taylor Whatley on 2018-09-28.
//

#include "QNemWindow.h"

#include "../Internal.h"
#include "../Timer.h"
#include "../Errors.h"

#include <fstream>
#include <iostream>

#ifndef TIMER_TYPE
#define QT_TIMER_TYPE Qt::CoarseTimer
#endif

char* loadSource(string path, int* length) {
    std::ifstream stream(path, std::ios::in | std::ios::ate);
    if (!stream.good()) throw Nem::ShaderNotFoundException(path);
    *length = (int)stream.tellg();
    char* shader = new char[*length];
    stream.seekg(0, std::ios::beg);
    stream.read(shader, *length);
    stream.close();

    return shader;
}

bool checkCompile(GLuint shader, string shaderName = "Shader") {
    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (!status) {
        GLint logLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
        vector<char> log((unsigned long)logLength);
        glGetShaderInfoLog(shader, logLength, nullptr, &log[0]);
        std::cout << shaderName << " compile error: " << std::endl << string(log.begin(), log.end()) << std::endl;
        return false;
    }
    return true;
}

bool checkLink(GLuint program, string programName = "Program") {
    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (!status) {
        GLint logLength;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
        vector<char> log((unsigned long)logLength);
        glGetProgramInfoLog(program, logLength, nullptr, &log[0]);
        std::cout << programName << " link error: " << std::endl << string(log.begin(), log.end()) << std::endl;
        return false;
    }
    return true;
}

bool QNemWindow::loadShaders() {
    GLuint mapVertex = glCreateShader(GL_VERTEX_SHADER);
    GLuint spriteVertex = glCreateShader(GL_VERTEX_SHADER);
    GLuint palettedFragment = glCreateShader(GL_FRAGMENT_SHADER);

    GLint mapVertexLength, spriteVertexLength, palettedFragmentLength;

    const char* mapVertexSource = loadSource("map.vert", &mapVertexLength);
    const char* spriteVertexSource = loadSource("sprite.vert", &spriteVertexLength);
    const char* palettedFragmentSource = loadSource("paletted.frag", &palettedFragmentLength);

    glShaderSource(mapVertex, 1, &mapVertexSource, &mapVertexLength);
    glShaderSource(spriteVertex, 1, &spriteVertexSource, &spriteVertexLength);
    glShaderSource(palettedFragment, 1, &palettedFragmentSource, &palettedFragmentLength);

    delete[] mapVertexSource;
    delete[] spriteVertexSource;
    delete[] palettedFragmentSource;

    glCompileShader(mapVertex);
    glCompileShader(spriteVertex);
    glCompileShader(palettedFragment);

    if (!checkCompile(mapVertex, "Map Vertex")) return false;
    if (!checkCompile(spriteVertex, "Sprite Vertex")) return false;
    if (!checkCompile(palettedFragment, "Paletted Fragment")) return false;

    backgroundProgram = glCreateProgram();
    glAttachShader(backgroundProgram, mapVertex);
    glAttachShader(backgroundProgram, palettedFragment);
    glLinkProgram(backgroundProgram);
    glDetachShader(backgroundProgram, mapVertex);
    glDetachShader(backgroundProgram, palettedFragment);

    if (!checkLink(backgroundProgram, "Background Program")) return false;

    spriteProgram = glCreateProgram();
    glAttachShader(spriteProgram, spriteVertex);
    glAttachShader(spriteProgram, palettedFragment);
    glLinkProgram(spriteProgram);
    glDetachShader(spriteProgram, spriteVertex);
    glDetachShader(spriteProgram, palettedFragment);

    if (!checkLink(spriteProgram, "Sprite Program")) return false;

    glDeleteShader(mapVertex);
    glDeleteShader(spriteVertex);
    glDeleteShader(palettedFragment);

    return true;
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

    if (!loadShaders()) exit(0);

    emulator.ppu->memory->palettes.setClearColor(0x00); // REMOVE LATER WHEN GRAPHICS_ONLY IS NOT DEFINED
    Nem::Color color = Nem::palette2C02[emulator.ppu->memory->palettes.getClearColor()];
    glClearColor(color.red, color.green, color.blue, 1);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint pTex[] = {
            0x16, 0x04,
            0x18, 0x09,
    };

    glGenTextures(1, &mainTexture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mainTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8UI, 2, 2, 0, GL_RED, GL_UNSIGNED_INT, pTex);

    glGenSamplers(1, &sampler);
    glBindSampler(0, sampler);
    glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, GL_REPEAT);

//    GLfloat triangle[] = {
//            1, 1,
//            1, 0,
//            0, 0,
//    };
//
//    glGenBuffers(1, &buffer);
//    glBindBuffer(GL_ARRAY_BUFFER, buffer);
//    glBufferData(GL_ARRAY_BUFFER, sizeof(triangle), triangle, GL_STATIC_READ);

    initialized = true;
}

void QNemWindow::clearColorListener(Byte value) {
    if (initialized) {
        Nem::Color color = Nem::palette2C02[value];
        glClearColor(color.red, color.green, color.blue, 1);
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
//    glBindBuffer(GL_ARRAY_BUFFER, buffer);
//    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
//    glEnableVertexAttribArray(0);
    glDrawArrays(GL_TRIANGLES, 0, 30 * 5 * 6);

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
    render();
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
    glDeleteTextures(1, &mainTexture);
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &buffer);
    glDeleteProgram(backgroundProgram);
    glDeleteProgram(spriteProgram);

    delete context;
}