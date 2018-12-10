#version 330 core

const vec2 vertexTypes[6] = vec2[](
    vec2(0, 0),
    vec2(1, 0),
    vec2(0, 1),
    vec2(1, 0),
    vec2(0, 1),
    vec2(1, 1)
);

const vec2 unit = vec2(1.0f / 256.0f, 1.0f / 240.0f);
const float tileSize = 8.0f;

uniform usampler1D oam;

flat out int paletteId;
out vec2 spriteCoord;
flat out ivec2 patternCoord;

const uint flipX = 64u;
const uint flipY = 128u;
const uint patternTableIndex = 128u;

bool isFlagSet(uint flags, uint flag) { return (flags & flag) == flag; }

void main() {
    int shapeId = gl_VertexID / 6;
    int vertexType = gl_VertexID % 6;

    vec2 vertex = vertexTypes[vertexType];

    uvec2 pos = uvec2(texelFetch(oam, 4 * shapeId + 3, 0).r, texelFetch(oam, 4 * shapeId + 0, 0).r);
    uint patternFlags = texelFetch(oam, 4 * shapeId + 1, 0).r;
    uint flags = texelFetch(oam, 4 * shapeId + 2, 0).r;

    int index = int((patternFlags & patternTableIndex) >> 7);
    int patternRef = int(patternFlags);

    paletteId = int(flags & 3u);
    spriteCoord = vertex;
    patternCoord = ivec2(0, patternRef * 8 + 0 * 8 * 256);

    if (isFlagSet(flags, flipX)) spriteCoord.x = 1 - spriteCoord.x;
    if (isFlagSet(flags, flipY)) spriteCoord.y = 1 - spriteCoord.y;

    vertex.x *= tileSize;
    vertex += pos;
    vertex * unit;

    vertex.x = (vertex.x * 2 - 1);
    vertex.y = (vertex.y * 2 - 1) * -1;

    gl_Position = vec4(vertex, 0, 1);
}
