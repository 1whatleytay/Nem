//
// Created by Taylor Whatley on 2018-10-03.
//

#include "QNemWindow.h"

#include <fstream>
#include <iostream>

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

void QNemWindow::loadPPUPalette(GLuint program, Nem::PPUPalette palette, string location) {
    GLint colorA = glGetUniformLocation(program, (location + ".colorA").c_str());
    GLint colorB = glGetUniformLocation(program, (location + ".colorB").c_str());
    GLint colorC = glGetUniformLocation(program, (location + ".colorC").c_str());

    glUseProgram(program);
    glUniform1ui(colorA, (GLuint)palette.colorA);
    glUniform1ui(colorB, (GLuint)palette.colorB);
    glUniform1ui(colorC, (GLuint)palette.colorC);
}

void QNemWindow::loadUniforms() {
    glUseProgram(backgroundProgram);

    uniformBkgPatternSampler = glGetUniformLocation(backgroundProgram, "patternTable");
    uniformBkgNameTableSampler = glGetUniformLocation(backgroundProgram, "nameTable");
    uniformBkgNameTableDrawIndex = glGetUniformLocation(backgroundProgram, "nameTableDrawIndex");
    uniformBkgPatternTableDrawIndex = glGetUniformLocation(backgroundProgram, "patternTableDraWIndex");

    glUniform1i(uniformBkgPatternSampler, 0);
    glUniform1i(uniformBkgNameTableSampler, 1);

    emulator.ppu->memory->palettes.setBackgroundPalette(0, 0, 0x16);
    emulator.ppu->memory->palettes.setBackgroundPalette(0, 1, 0x37);
    emulator.ppu->memory->palettes.setBackgroundPalette(0, 2, 0x18);
    loadPPUPalette(backgroundProgram, emulator.ppu->memory->palettes.getBackgroundPalette(0), "palettes[0]");
    loadPPUPalette(backgroundProgram, emulator.ppu->memory->palettes.getBackgroundPalette(1), "palettes[1]");
    loadPPUPalette(backgroundProgram, emulator.ppu->memory->palettes.getBackgroundPalette(2), "palettes[2]");
    loadPPUPalette(backgroundProgram, emulator.ppu->memory->palettes.getBackgroundPalette(3), "palettes[3]");
}

bool QNemWindow::loadShaders() {
    GLuint mapVertex = glCreateShader(GL_VERTEX_SHADER);
    //GLuint spriteVertex = glCreateShader(GL_VERTEX_SHADER);
    GLuint palettedFragment = glCreateShader(GL_FRAGMENT_SHADER);

    GLint mapVertexLength, spriteVertexLength, palettedFragmentLength;

    const char* mapVertexSource = loadSource("map.vert", &mapVertexLength);
    //const char* spriteVertexSource = loadSource("sprite.vert", &spriteVertexLength);
    const char* palettedFragmentSource = loadSource("paletted.frag", &palettedFragmentLength);

    glShaderSource(mapVertex, 1, &mapVertexSource, &mapVertexLength);
    //glShaderSource(spriteVertex, 1, &spriteVertexSource, &spriteVertexLength);
    glShaderSource(palettedFragment, 1, &palettedFragmentSource, &palettedFragmentLength);

    delete[] mapVertexSource;
    //delete[] spriteVertexSource;
    delete[] palettedFragmentSource;

    glCompileShader(mapVertex);
    //glCompileShader(spriteVertex);
    glCompileShader(palettedFragment);

    if (!checkCompile(mapVertex, "Map Vertex")) return false;
    //if (!checkCompile(spriteVertex, "Sprite Vertex")) return false;
    if (!checkCompile(palettedFragment, "Paletted Fragment")) return false;

    backgroundProgram = glCreateProgram();
    glAttachShader(backgroundProgram, mapVertex);
    glAttachShader(backgroundProgram, palettedFragment);
    glLinkProgram(backgroundProgram);
    glDetachShader(backgroundProgram, mapVertex);
    glDetachShader(backgroundProgram, palettedFragment);

    if (!checkLink(backgroundProgram, "Background Program")) return false;

//    spriteProgram = glCreateProgram();
//    glAttachShader(spriteProgram, spriteVertex);
//    glAttachShader(spriteProgram, palettedFragment);
//    glLinkProgram(spriteProgram);
//    glDetachShader(spriteProgram, spriteVertex);
//    glDetachShader(spriteProgram, palettedFragment);
//
//    if (!checkLink(spriteProgram, "Sprite Program")) return false;

    glDeleteShader(mapVertex);
    //glDeleteShader(spriteVertex);
    glDeleteShader(palettedFragment);

    return true;
}