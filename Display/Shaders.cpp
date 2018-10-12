//
// Created by Taylor Whatley on 2018-10-06.
//

#include "Display.h"

#include "../PPU/PPU.h"

#include <fstream>
#include <iostream>

namespace Nem {
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

    vector<GLuint> Display::makePatternData(PPU *ppu) {
        int spriteCount = 0x2000 / 16;
        vector<GLuint> data = vector<GLuint>((unsigned long)(spriteCount * 8 * 8));
        for (int a = 0; a < spriteCount; a++) {
            for (int b = 0; b < 8; b++) {
                Byte layer0 = ppu->memory->getByte(PPUMemoryRegion::PatternTable0 + a * 16 + b);
                Byte layer1 = ppu->memory->getByte(PPUMemoryRegion::PatternTable0 + a * 16 + 8 + b);
                for (int c = 0; c < 8; c++) {
                    Byte bit = (Byte)0b10000000 >> c;
                    data[(a * 8 * 8) + b * 8 + c] =
                            (GLuint)(
                                    (((layer1 & bit) == bit) << 1) +
                                    ((layer0 & bit) == bit));
                }
            }
        }
        return data;
    }

    vector<GLuint> Display::makeNameTableData(Nem::PPU* ppu) {
        vector<GLuint> data = vector<GLuint>(kilobyte(2));
        for (int a = 0; a < kilobyte(1); a++) {
            data[a] =
                    ppu->memory->getByte(
                            PPUMemoryRegion::NameTable0 + ppu->memory->getNameTableByIndex(0) * 0x400 + a);
            data[a + kilobyte(1)] =
                    ppu->memory->getByte(
                            PPUMemoryRegion::NameTable0 + ppu->memory->getNameTableByIndex(1) * 0x400 + a);
        }
        return data;
    }

    vector<GLuint> Display::makeOAMData(PPU* ppu) {
        vector<GLuint> data = vector<GLuint>(64 * 4);
        for (int a = 0; a < 64 * 4; a++) data[a] = ppu->memory->oam[a];
        return data;
    }

    bool Display::loadShaders() {
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

    void Display::loadUniforms() {
        glUseProgram(backgroundProgram);

        uniformBkgPatternSampler = glGetUniformLocation(backgroundProgram, "patternTable");
        uniformBkgNameTableSampler = glGetUniformLocation(backgroundProgram, "nameTable");
        uniformBkgPaletteSampler = glGetUniformLocation(backgroundProgram, "palette");
        uniformBkgNameTableDrawIndex = glGetUniformLocation(backgroundProgram, "nameTableDrawIndex");
        uniformBkgPatternTableDrawIndex = glGetUniformLocation(backgroundProgram, "patternTableDrawIndex");
        uniformBkgScrollX = glGetUniformLocation(backgroundProgram, "scrollX");
        uniformBkgScrollY = glGetUniformLocation(backgroundProgram, "scrollY");

        glUniform1i(uniformBkgPatternSampler, 0);
        glUniform1i(uniformBkgNameTableSampler, 1);
        glUniform1i(uniformBkgPaletteSampler, 2);
        glUniform1i(uniformBkgScrollX, 0);
        glUniform1i(uniformBkgScrollY, 0);

        glUseProgram(spriteProgram);

        uniformSprPatternSampler = glGetUniformLocation(spriteProgram, "patternTable");
        uniformSprPaletteSampler = glGetUniformLocation(spriteProgram, "palette");
        uniformSprOAMSampler = glGetUniformLocation(spriteProgram, "oam");
        uniformSprPatternTableDrawIndex = glGetUniformLocation(spriteProgram, "patternTableDrawIndex");

        glUniform1i(uniformSprPatternSampler, 0);
        glUniform1i(uniformSprPaletteSampler, 2);
        glUniform1i(uniformSprOAMSampler, 3);
    }
}