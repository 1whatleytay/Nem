#version 330 core

uniform sampler1D palette;
uniform usampler2D patternTable;

flat in int paletteId;
in vec2 spriteCoord;
flat in ivec2 patternCoord;

out vec4 color;

struct PPUPalette { uint colorA, colorB, colorC; };
uniform PPUPalette palettes[4];

uint getPaletteColor(PPUPalette palette, uint index) {
    switch (index) {
    case 1:
        return palette.colorA;
    case 2:
        return palette.colorB;
    case 3:
        return palette.colorC;
    }
    return uint(0x41);
}

void main() {
    ivec2 patternLoc = patternCoord;
    patternLoc.x += int(spriteCoord.x * 8.0f);
    patternLoc.y += int(spriteCoord.y * 8.0f);
    uint paletteIndex = texelFetch(patternTable, patternLoc, 0).r;
    uint value = getPaletteColor(palettes[paletteId], paletteIndex);
    if (paletteIndex == uint(0)) value = uint(0x41);
    if (value > uint(0x40)) discard;
    color = vec4(texelFetch(palette, int(value), 0).rgb, 1);
}
