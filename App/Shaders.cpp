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
        string vertexSource = loadFromFile("vertex.vert"), fragmentSource = loadFromFile("fragment.frag");
        const GLint lVertexSource = (GLint)vertexSource.size(), lFragmentSource = (GLint)fragmentSource.size();
        const GLchar *cVertexSource = vertexSource.c_str(), *cFragmentSource = fragmentSource.c_str();

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
    }

    void Display::checkEdits() {
        if (!ppu->memory.edits.nameTable[0].ranges.empty()) {
            glBindBuffer(GL_ARRAY_BUFFER, nameTable[0]);

            for (const Ranges::SubRange& range : ppu->memory.edits.nameTable[0].ranges) {
                GLfloat data[6 * 30 * 32 * 2];

                for (int x = 0; x < 32; x++) {
                    for (int y = 0; y < 30; y++) {
                        int index = 6 * 2 * (x + 32 * y);
                        data[index + 0] = ((float) x / 32.0f) * 2 - 1;
                        data[index + 1] = ((float) y / 30.0f) * -2 + 1;

                        data[index + 2] = ((float) x / 32.0f) * 2 - 1;
                        data[index + 3] = ((float) (y + 1) / 30.0f) * -2 + 1;

                        data[index + 4] = ((float) (x + 1) / 32.0f) * 2 - 1;
                        data[index + 5] = ((float) y / 30.0f) * -2 + 1;

                        data[index + 6] = ((float) (x + 1) / 32.0f) * 2 - 1;
                        data[index + 7] = ((float) y / 30.0f) * -2 + 1;

                        data[index + 8] = ((float) x / 32.0f) * 2 - 1;
                        data[index + 9] = ((float) (y + 1) / 30.0f) * -2 + 1;

                        data[index + 10] = ((float) (x + 1) / 32.0f) * 2 - 1;
                        data[index + 11] = ((float) (y + 1) / 30.0f) * -2 + 1;
                    }
                }

                glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(data), data);
            }

            ppu->memory.edits.nameTable[0].ranges.clear();
        }
    }
}