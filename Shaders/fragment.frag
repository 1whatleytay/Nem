#version 330 core

uniform sampler1D palette;
uniform isampler2D pattern;

flat in int paletteId;
in vec2 texCoord;

out vec4 outColor;

uniform int paletteIndex;
uniform ivec3 paletteRam[8];

void main() {
    int tex = texture(pattern, texCoord).r - 1;
    if (tex == -1) discard;
	outColor = texelFetch(palette, paletteRam[paletteId + paletteIndex * 4][tex], 0);
}