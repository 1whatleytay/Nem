#version 330 core

layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 tex;

flat out int color;
out vec2 texCoord;

void main() {
    color = (gl_VertexID / 6) % 0x40;
    texCoord = tex;
	gl_Position = vec4(pos, 0.0, 1.0);
}