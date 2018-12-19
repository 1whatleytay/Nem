#version 330 core

uniform sampler1D palette;
uniform isampler2D pattern;

flat in int color;
in vec2 texCoord;

out vec4 outColor;

const int paletteRam[] = int[]( 0x05, 0x17, 0x29, 0x34 );

void main() {
	outColor = texelFetch(palette, paletteRam[texture(pattern, texCoord).r], 0);
}