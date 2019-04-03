//
// Created by Taylor Whatley on 2018-12-13.
//

#include "App.h"
#include "../PPU/PPU.h"

#include <fstream>
#include <iostream>

namespace Nem {
    string loadFromFile(const string& file) {
        std::ifstream stream = std::ifstream(file, std::ios::ate | std::ios::in);
        if (!stream.good()) throw ShaderNotFoundException(file);
        vector<char> data((unsigned long)stream.tellg());
        stream.seekg(0, std::ios::beg);
        stream.read(&data[0], data.size());
        stream.close();

        return string(data.begin(), data.end());
    }

    bool checkShaderLog(GLuint shader, const string& name) {
        GLint status;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

        if (!status) {
            int logLength;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
            vector<char> log((unsigned long)logLength);
            glGetShaderInfoLog(shader, logLength, &logLength, &log[0]);
            std::cout << "Could not compile \"" << name << "\":\n"
            << string(log.begin(), log.end()) << std::endl;
            return false;
        }

        return true;
    }

    bool checkProgramLog(GLuint program) {
        GLint status;
        glGetProgramiv(program, GL_LINK_STATUS, &status);

        if (!status) {
            int logLength;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
            vector<char> log((unsigned long)logLength);
            glGetProgramInfoLog(program, logLength, &logLength, &log[0]);
            std::cout << "Could not link program:\n"
            << string(log.begin(), log.end()) << std::endl;
            return false;
        }

        return true;
    }

    void Display::loadShaders() {
        string vertexSource = loadFromFile("vertex.vert"),
            fragmentSource = loadFromFile("fragment.frag");
        const GLint lVertexSource = (GLint)vertexSource.size(),
            lFragmentSource = (GLint)fragmentSource.size();
        const GLchar *cVertexSource = vertexSource.c_str(),
            *cFragmentSource = fragmentSource.c_str();

        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

        glShaderSource(vertexShader, 1, &cVertexSource, &lVertexSource);
        glCompileShader(vertexShader);
        checkShaderLog(vertexShader, "vertex.vert");

        glShaderSource(fragmentShader, 1, &cFragmentSource, &lFragmentSource);
        glCompileShader(fragmentShader);
        checkShaderLog(fragmentShader, "fragment.frag");

        program = glCreateProgram();
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);
        glLinkProgram(program);
        checkProgramLog(program);

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        glUseProgram(program);
        GLint paletteSampler = glGetUniformLocation(program, "palette");
        GLint patternSampler = glGetUniformLocation(program, "pattern");
        glUniform1i(paletteSampler, 0);
        glUniform1i(patternSampler, 1);

        uniOffsetX = glGetUniformLocation(program, "offsetX");
        glUniform1i(uniOffsetX, 0);

        uniPaletteRamIndex = glGetUniformLocation(program, "paletteIndex");
        glUniform1i(uniPaletteRamIndex, 0);

        for (int a = 0; a < 4; a++) {
            uniBkgPaletteRam[a] = glGetUniformLocation(program, ("paletteRam[" + std::to_string(a) + "]").c_str());
        }
        for (int a = 0; a < 4; a++) {
            uniSprPaletteRam[a] = glGetUniformLocation(program, ("paletteRam[" + std::to_string(a + 4) + "]").c_str());
        }
    }

    void Display::checkEdits() {
        ppu->memory.edits.mutex.lock();
        ppu->memory.checkNeedsRefresh();
        for (int a = 0; a < 2; a++) {
            if (!ppu->memory.edits.nameTable[a].ranges.empty()) {
                glBindVertexArray(nameTableVAOs[a]);
                glBindBuffer(GL_ARRAY_BUFFER, nameTableBuffers[a]);

                for (const Ranges::SubRange &range : ppu->memory.edits.nameTable[a].ranges) {
                    vector<Vertex> data = vector<Vertex>((unsigned) (range.count * 6));

                    for (int r = 0; r < range.count; r++) {
                        int rangeBegin = r + range.start, index = r * 6;
                        int x = rangeBegin % 32, y = rangeBegin / 32;
                        float x1 = (float) x / 32.0f, y1 = (float) y / 30.0f;
                        float x2 = (float) (x + 1) / 32.0f, y2 = (float) (y + 1) / 30.0f;
                        int val = ppu->memory.getByte(PPUMemory::regionIndex(NameTables, a) + (Address) rangeBegin);
                        float texStart = (float) (val) / 256.0f, texEnd = (float) (val + 1) / 256.0f;
                        Address pos = (Address)(PPUMemory::regionIndex(AttributeTables, a) + y / 4 * 8 + x / 4);
                        GLint paletteId = shiftAttribute(ppu->memory.getByte(pos), x, y) & 0b00000011;

                        data[index + 0] = { x1 * 2 - 1, y1 * -2 + 1, 0.0f, texStart, paletteId };
                        data[index + 1] = { x1 * 2 - 1, y2 * -2 + 1, 0.0f, texEnd, paletteId };
                        data[index + 2] = { x2 * 2 - 1, y1 * -2 + 1, 1.0f, texStart, paletteId };
                        data[index + 3] = { x2 * 2 - 1, y1 * -2 + 1, 1.0f, texStart, paletteId };
                        data[index + 4] = { x1 * 2 - 1, y2 * -2 + 1, 0.0f, texEnd, paletteId };
                        data[index + 5] = { x2 * 2 - 1, y2 * -2 + 1, 1.0f, texEnd, paletteId };
                    }
                    
                    glBufferSubData(GL_ARRAY_BUFFER, range.start * sizeof(Vertex) * 6,
                            data.size() * sizeof(Vertex), &data[0]);
                }

                ppu->memory.edits.nameTable[a].ranges.clear();
            }
        }

        if (!ppu->memory.edits.oam.ranges.empty()) {
            glBindVertexArray(oamVAO);
            glBindBuffer(GL_ARRAY_BUFFER, oamBuffer);

            for (const Ranges::SubRange &range : ppu->memory.edits.oam.ranges) {
                vector<Vertex> data = vector<Vertex>((unsigned) (range.count * 6));

                for (int r = 0; r < range.count; r++) {
                    int rangeBegin = r + range.start, index = r * 6;
                    int x = ppu->memory.getOAM((Byte) (rangeBegin * 4 + 3));
                    int y = ppu->memory.getOAM((Byte) (rangeBegin * 4 + 0));
                    float x1 = (float) x / 256.0f, y1 = (float) y / 240.0f;
                    float x2 = (float) (x + 8) / 256.0f, y2 = (float) (y + 8) / 240.0f;
                    Byte flags = ppu->memory.getOAM((Byte) (rangeBegin * 4 + 2));
                    int paletteId = flags & 0b00000011;
                    bool flipX = (flags & 0b01000000) == 0b01000000, flipY = (flags & 0b10000000) == 0b10000000;
                    int texPick = ppu->memory.getOAM((Byte) (rangeBegin * 4 + 1));
                    float texStart = (float) (texPick + (flipY ? 1 : 0)) / 256.0f;
                    float texEnd = (float) (texPick + (flipY ? 0 : 1)) / 256.0f;
                    float base1 = flipX ? 1.0f : 0.0f, base2 = flipX ? 0.0f : 1.0f;

                    data[index + 0] = { x1 * 2 - 1, y1 * -2 + 1, base1, texStart, paletteId };
                    data[index + 1] = { x1 * 2 - 1, y2 * -2 + 1, base1, texEnd, paletteId };
                    data[index + 2] = { x2 * 2 - 1, y1 * -2 + 1, base2, texStart, paletteId };
                    data[index + 3] = { x2 * 2 - 1, y1 * -2 + 1, base2, texStart, paletteId };
                    data[index + 4] = { x1 * 2 - 1, y2 * -2 + 1, base1, texEnd, paletteId };
                    data[index + 5] = { x2 * 2 - 1, y2 * -2 + 1, base2, texEnd, paletteId };
                }

                glBufferSubData(GL_ARRAY_BUFFER, range.start * sizeof(Vertex) * 6,
                        data.size() * sizeof(Vertex), &data[0]);

                ppu->memory.edits.oam.ranges.clear();
            }
        }

        for (int a = 0; a < 2; a++) {
            if (!ppu->memory.edits.patternTable[a].ranges.empty()) {
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, patternTable[a]);

                for (const Ranges::SubRange &range : ppu->memory.edits.patternTable[a].ranges) {
                    vector<GLbyte> data = vector<GLbyte>((unsigned) (range.count * 8 * 8));

                    for (Address r = 0; r < range.count; r++) {
                        for (Address b = 0; b < 8; b++) {
                            Byte layer0 = ppu->memory.getByte(
                                    PPUMemory::regionIndex(PatternTables, a) + r * (Address) 16 + b);
                            Byte layer1 = ppu->memory.getByte(
                                    PPUMemory::regionIndex(PatternTables, a) + r * (Address) 16 +
                                    (Address) 8 + b);
                            for (int c = 0; c < 8; c++) {
                                Byte bit = (Byte) 0b10000000 >> c;
                                data[(r * 8 * 8) + b * 8 + c] =
                                        (GLbyte) (
                                                (((layer1 & bit) == bit) << 1) +
                                                ((layer0 & bit) == bit));
                            }
                        }
                    }

                    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, range.start * 8, 8, range.count * 8,
                                    GL_RED_INTEGER, GL_BYTE, &data[0]);
                }

                ppu->memory.edits.patternTable[a].ranges.clear();
            }
        }

        if (ppu->memory.edits.paletteRam) {
            Color clearColor = palette2C02[ppu->memory.palettes.clearColor];
            glClearColor(clearColor.red, clearColor.green, clearColor.blue, 1);
            for (int a = 0; a < 4; a++) {
                glUniform3i(uniBkgPaletteRam[a],
                        ppu->memory.palettes.background[a].colorA,
                        ppu->memory.palettes.background[a].colorB,
                        ppu->memory.palettes.background[a].colorC);
            }
            for (int a = 0; a < 4; a++) {
                glUniform3i(uniSprPaletteRam[a],
                            ppu->memory.palettes.sprite[a].colorA,
                            ppu->memory.palettes.sprite[a].colorB,
                            ppu->memory.palettes.sprite[a].colorC);
            }
            ppu->memory.edits.paletteRam = false;
        }

        ppu->memory.edits.mutex.unlock();
    }
}