//
// Created by Taylor Whatley on 2018-09-28.
//

#ifndef NEM_QNEMWINDOW_H
#define NEM_QNEMWINDOW_H

#include "../Nem.h"

#include <QTimer>
#include <QWindow>
#include <QOpenGLFunctions>

#include <thread>

class QNemWindow: public QWindow, protected QOpenGLFunctions {
    Q_OBJECT

    // OpenGL and Initialization
    QOpenGLContext* context = nullptr;
    GLuint backgroundProgram, spriteProgram;
    GLuint buffer;
    GLuint mainTexture;
    GLuint sampler;
    GLuint vao;
    bool initialized = false;

    int timer;

    // Emulator
    Nem::Emulator emulator;

    std::thread *cpuThread = nullptr;

    void clearColorListener(Byte value);
    void backgroundListener(int index, Nem::PPUPalette palette);
    void spriteListener(int index, Nem::PPUPalette palette);

    bool loadShaders();
    void init();
protected:
    bool event(QEvent* event) override;
    void timerEvent(QTimerEvent* event) override;

public:
    void render();

    explicit QNemWindow(string romPath);
    ~QNemWindow() override;
};

#endif //NEM_QNEMWINDOW_H
