//
// Created by Taylor Whatley on 2018-12-13.
//

#include "App.h"
#include "../PPU/PPU.h"

#include <fstream>
#include <iostream>

namespace Nem {
    void checkDebugBinding(string name) {
        GLuint buffer;
        glGetIntegerv(GL_ARRAY_BUFFER_BINDING, (GLint*)&buffer);

        std::cout << name << ": " << buffer << std::endl;
    }

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

        for (int a = 0; a < 4; a++) {
            uniPaletteRam[a] = glGetUniformLocation(program, ("paletteRam[" + std::to_string(a) + "]").c_str());
        }
    }

    void Display::checkEdits() {
        checkGL("In.");
        ppu->memory.edits.mutex.lock();
        ppu->memory.checkNeedsRefresh();
        for (int a = 0; a < 1; a++) {
            if (!ppu->memory.edits.nameTable[a].ranges.empty()) {
                std::cout << "Update NameTable" << a << std::endl;
                glBindBuffer(GL_ARRAY_BUFFER, 4);
                checkGL("PLS ERROR");

                for (const Ranges::SubRange &range : ppu->memory.edits.nameTable[a].ranges) {
                    vector<Vertex> data = vector<Vertex>((unsigned) (range.count * 6));

                    for (int r = 0; r < range.count; r++) {
                        int rangeBegin = r + range.start, index = r * 6;
                        int x = rangeBegin % 32, y = rangeBegin / 32;
                        float x1 = (float) x / 32.0f, y1 = (float) y / 30.0f;
                        float x2 = (float) (x + 1) / 32.0f, y2 = (float) (y + 1) / 30.0f;
                        int val = ppu->memory.getByte(PPUMemory::regionIndex(NameTables, a) + (Address) rangeBegin);
//                        std::cout << makeHex(PPUMemory::regionIndex(NameTables, a) + (Address) rangeBegin) << ": " << val << std::endl;
                        //std::cout << "Index: " << a << " " << makeHex(PPUMemory::regionIndex(NameTables, a) + (Address) rangeBegin) << std::endl;
                        float texStart = (float) (val) / 256.0f, texEnd = (float) (val + 1) / 256.0f;
                        int shift = (y / 2 % 2) * 4 + (x / 2 % 2) * 2;
                        Address pos = (Address)(PPUMemory::regionIndex(AttributeTables, a) + y / 4 * 8 + x / 4);
                        GLint paletteId = ppu->memory.getByte(pos) >> shift & 0b00000011;

                        data[index + 0] = { x1 * 2 - 1, y1 * -2 + 1, 0.0f, texStart, paletteId };
                        data[index + 1] = { x1 * 2 - 1, y2 * -2 + 1, 0.0f, texEnd, paletteId };
                        data[index + 2] = { x2 * 2 - 1, y1 * -2 + 1, 1.0f, texStart, paletteId };
                        data[index + 3] = { x2 * 2 - 1, y1 * -2 + 1, 1.0f, texStart, paletteId };
                        data[index + 4] = { x1 * 2 - 1, y2 * -2 + 1, 0.0f, texEnd, paletteId };
                        data[index + 5] = { x2 * 2 - 1, y2 * -2 + 1, 1.0f, texEnd, paletteId };
                    }
                    glBindBuffer(GL_ARRAY_BUFFER, 0);
                    glBindBuffer(GL_ARRAY_BUFFER, nameTables[1]);

                    checkDebugBinding("Writing to");

                    glBufferSubData(GL_ARRAY_BUFFER, range.start * sizeof(Vertex) * 6,
                                    data.size() * sizeof(Vertex), &data[0]);
                }

                ppu->memory.edits.nameTable[a].ranges.clear();
            }
        }
        checkGL("NameTable");

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

                ppu->memory.edits.patternTable[0].ranges.clear();
            }
        }

        if (ppu->memory.edits.paletteRam) {
            Color clearColor = palette2C02[ppu->memory.palettes.clearColor];
            glClearColor(clearColor.red, clearColor.green, clearColor.blue, 1);
            for (int a = 0; a < 4; a++) {
                glUniform3i(uniPaletteRam[a],
                        ppu->memory.palettes.background[a].colorA,
                        ppu->memory.palettes.background[a].colorB,
                        ppu->memory.palettes.background[a].colorC);
            }
            ppu->memory.edits.paletteRam = false;
        }

        ppu->memory.edits.mutex.unlock();
        checkGL("Check Edits");
    }
}