#version 330 core

uniform sampler1D palette;
uniform sampler2D pattern;

flat in int color;
out vec4 outColor;

void main() {
	outColor = texelFetch(palette, color, 0);
}