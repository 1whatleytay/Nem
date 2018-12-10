//
// Created by Taylor Whatley on 2018-10-06.
//

#include "App.h"

#include "../PPU/PPU.h"

#include <fstream>
#include <iostream>

namespace Nem {
    //const int spriteCount = 0x2000 / 16;

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

    Color makeGrayscale(Color color) {
        float baseColor = color.red * 0.299f + color.green * 0.587f + color.green * 0.113f;
        return { baseColor, baseColor, baseColor };
    }

    void loadPPUPalette(GLuint program, Nem::PPUPalette palette, string location) {
        GLint colorA = glGetUniformLocation(program, (location + ".colorA").c_str());
        GLint colorB = glGetUniformLocation(program, (location + ".colorB").c_str());
        GLint colorC = glGetUniformLocation(program, (location + ".colorC").c_str());

        glUseProgram(program);
        glUniform1i(colorA, (int)palette.colorA);
        glUniform1i(colorB, (int)palette.colorB);
        glUniform1i(colorC, (int)palette.colorC);
    }

    vector<GLint> makePatternData(PPU *ppu, Address start, Address count) {
        vector<GLint> data = vector<GLint>((unsigned long)(count * 8 * 8));
        for (int a = 0; a < count; a++) {
            for (int b = 0; b < 8; b++) {
                Byte layer0 = ppu->memory->getByte(PPUMemoryRegion::PatternTable0 + (a + start) * 16 + b);
                Byte layer1 = ppu->memory->getByte(PPUMemoryRegion::PatternTable0 + (a + start) * 16 + 8 + b);
                for (int c = 0; c < 8; c++) {
                    Byte bit = (Byte)0b10000000 >> c;
                    data[((a + start) * 8 * 8) + b * 8 + c] =
                                    (((layer1 & bit) == bit) << 1) +
                                    ((layer0 & bit) == bit);
                }
            }
        }
        return data;
    }

    vector<GLint> makeNameTableData(Nem::PPU* ppu, Address start, Address count) {
        vector<GLint> data = vector<GLint>(count);
        for (Address a = 0; a < count; a++) data[a] = ppu->memory->getByte(a + start);
        return data;
    }

    vector<GLuint> makeOAMData(PPU* ppu, Byte start, Byte count) {
        vector<GLuint> data = vector<GLuint>(64 * 4);
        for (Byte a = 0; a < count; a++) data[a] = ppu->memory->getOAM(a + start);
        return data;
    }

    void Display::refreshPatternTable() {
        for (Ranges::SubRange range : ppu->memory->edits.patternTable.ranges) {
            vector<GLint> patternTableTex = makePatternData(ppu, (Address)range.start, (Address)range.count);

            glActiveTexture(GL_TEXTURE0 + 0);
            glBindTexture(GL_TEXTURE_2D, patternTexture);
            glTexSubImage2D(GL_TEXTURE_2D, 0,
                    0, range.start * 8, 8, range.count * 8,
                    GL_RED_INTEGER, GL_UNSIGNED_INT, &patternTableTex[0]);
        }
    }
    void Display::refreshNameTable() {
        for (Ranges::SubRange range : ppu->memory->edits.nameTable.ranges) {
            vector<GLint> nameTableTex = makeNameTableData(ppu, (Address)range.start, (Address)range.count);

            glActiveTexture(GL_TEXTURE0 + 1);
            glBindTexture(GL_TEXTURE_1D, nameTableTexture);
            glTexSubImage1D(GL_TEXTURE_1D, 0,
                    range.start - PPUMemoryRegion::PatternTable0, range.count,
                    GL_RED_INTEGER, GL_INT, &nameTableTex[0]);
        }
    }
    void Display::refreshPaletteRam() {
        Color clearColor = palette2C02[ppu->memory->getByte(0x3f00) % 64];
        if (ppu->isMaskSet(PPURegisters::MaskFlags::Grayscale))
            clearColor = makeGrayscale(clearColor);
        glClearColor(clearColor.red, clearColor.green, clearColor.blue, 1);

        loadPPUPalette(backgroundProgram, ppu->memory->palettes.background[0], "palettes[0]");
        loadPPUPalette(backgroundProgram, ppu->memory->palettes.background[1], "palettes[1]");
        loadPPUPalette(backgroundProgram, ppu->memory->palettes.background[2], "palettes[2]");
        loadPPUPalette(backgroundProgram, ppu->memory->palettes.background[3], "palettes[3]");

        loadPPUPalette(spriteProgram, ppu->memory->palettes.sprite[0], "palettes[0]");
        loadPPUPalette(spriteProgram, ppu->memory->palettes.sprite[1], "palettes[1]");
        loadPPUPalette(spriteProgram, ppu->memory->palettes.sprite[2], "palettes[2]");
        loadPPUPalette(spriteProgram, ppu->memory->palettes.sprite[3], "palettes[3]");
    }

    void Display::refreshOAM() {
        for (Ranges::SubRange range : ppu->memory->edits.oam.ranges) {
            vector<GLuint> oamTex = makeOAMData(ppu, (Byte)range.start, (Byte)range.count);

            glActiveTexture(GL_TEXTURE0 + 3);
            glBindTexture(GL_TEXTURE_1D, oamTexture);
            glTexSubImage1D(GL_TEXTURE_1D, 0,
                    range.start, range.count,
                    GL_RED_INTEGER, GL_UNSIGNED_INT, &oamTex[0]);
        }
    }

    void Display::refreshRegisters() {
        glUniform1i(uniformBkgGrayscale, ppu->isMaskSet(PPURegisters::MaskFlags::Grayscale));
        glUniform1i(uniformSprGrayscale, ppu->isMaskSet(PPURegisters::MaskFlags::Grayscale));
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
        uniformBkgGrayscale = glGetUniformLocation(backgroundProgram, "grayscale");

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
        uniformSprGrayscale = glGetUniformLocation(backgroundProgram, "grayscale");

        glUniform1i(uniformSprPatternSampler, 0);
        glUniform1i(uniformSprPaletteSampler, 2);
        glUniform1i(uniformSprOAMSampler, 3);
    }
}