#version 330 core

uniform sampler1D palette;
uniform isampler2D patternTable;

//uniform bool grayscale = false;

flat in int paletteId;
in vec2 spriteCoord;
flat in ivec2 patternCoord;

out vec4 color;

struct PPUPalette { int colorA, colorB, colorC; };
uniform PPUPalette palettes[4];

int getPaletteColor(PPUPalette palette, int index) {
    switch (index) {
    case 1:
        return palette.colorA;
    case 2:
        return palette.colorB;
    case 3:
        return palette.colorC;
    default:
        break;
    }
    return 0x41;
}

void main() {
    ivec2 patternLoc = patternCoord;
    patternLoc.x += int(spriteCoord.x * 8.0f);
    patternLoc.y += int(spriteCoord.y * 8.0f);
    int paletteIndex = texelFetch(patternTable, patternLoc, 0).r;
    int value = getPaletteColor(palettes[paletteId], paletteIndex);
    if (paletteIndex == 0 || value > 0x40) discard;
    vec3 process = texelFetch(palette, value, 0).rgb;
//    if (grayscale) {
//        float baseColor = dot(process, vec3(0.299, 0.587, 0.114));
//        process = vec3(baseColor);
//    }
    color = vec4(process, 1);
}
