#version 330 core

const vec2 vertexTypes[6] = vec2[](
    vec2(0, 0),
    vec2(1, 0),
    vec2(0, 1),
    vec2(1, 0),
    vec2(0, 1),
    vec2(1, 1)
);

struct Metadata {
    float unitX, unitY;
    float tileWidth, tileHeight;
};
const Metadata meta = Metadata(1.0f / 256.0f, 1.0f / 240.0f, 8.0f, 8.0f);

uniform usampler1D oam;

flat out int paletteId;
out vec2 spriteCoord;
flat out ivec2 patternCoord;

const uint flipX = uint(64);
const uint flipY = uint(128);
const uint patternTableIndex = uint(128);

bool isFlagSet(uint flags, uint flag) { return (flags & flag) == flag; }

void main() {
    int shapeId = gl_VertexID / 6;
    int vertexType = gl_VertexID % 6;

    vec2 vertex = vertexTypes[vertexType];

    uint xPos = texelFetch(oam, 4 * shapeId + 3, 0).r, yPos = texelFetch(oam, 4 * shapeId + 0, 0).r;
    uint patternFlags = texelFetch(oam, 4 * shapeId + 1, 0).r;
    uint flags = texelFetch(oam, 4 * shapeId + 2, 0).r;

    int index = int((patternFlags & patternTableIndex) >> 7);
    int patternRef = int(patternFlags);

    paletteId = int(flags & uint(3));
    spriteCoord = vertex;
    patternCoord = ivec2(0, patternRef * 8 + index * 8 * 256);

    vertex.x *= meta.tileWidth;
    vertex.y *= meta.tileHeight;

    vertex.x += float(xPos);
    vertex.y += float(yPos);

    vertex.x *= meta.unitX;
    vertex.y *= meta.unitY;

    vertex.x = (vertex.x * 2 - 1);
    vertex.y = (vertex.y * 2 - 1) * -1;

    gl_Position = vec4(vertex, 0, 1);
}
