#version 330 core

layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 tex;
layout(location = 2) in int id;

flat out int paletteId;
out vec2 texCoord;

void main() {
    paletteId = id;
    texCoord = tex;
	gl_Position = vec4(pos, 0.0, 1.0);
}