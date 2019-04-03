#version 330 core

layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 tex;
layout(location = 2) in int id;

uniform int offsetX;

flat out int paletteId;
out vec2 texCoord;

void main() {
    paletteId = id;
    texCoord = tex;
    vec2 point = pos;
    point.x += offsetX / 128.0f;
	gl_Position = vec4(point, 0.0, 1.0);
}