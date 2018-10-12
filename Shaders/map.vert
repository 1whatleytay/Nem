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
    int mapWidth, mapHeight;
    float tileUnitWidth, tileUnitHeight;
};
const Metadata meta = Metadata(32, 30, 1.0f / 32.0f, 1.0f / 30.0f);

uniform usampler1D nameTable;

uniform int nameTableDrawIndex;
uniform int patternTableDrawIndex;
uniform int scrollX, scrollY;

flat out int paletteId;
out vec2 spriteCoord;
flat out ivec2 patternCoord;

void main() {
    int shapeId = gl_VertexID / 6;
    int vertexType = gl_VertexID % 6;

    int tilePosX = shapeId % meta.mapWidth, tilePosY = shapeId / meta.mapWidth;

    vec2 vertex = vertexTypes[vertexType];

    int patternRef = int(texelFetch(nameTable, shapeId + 1024 * nameTableDrawIndex, 0).r);

    spriteCoord = vertex;
    patternCoord = ivec2(0, patternRef * 8 + 256 * 8 * patternTableDrawIndex);

    uint attributeFetch = texelFetch(nameTable, 960 + tilePosX / 4 + tilePosY / 4 * 8 + 1024 * nameTableDrawIndex, 0).r;
    attributeFetch = attributeFetch >> (4 * (tilePosY / 2 % 2)) + (2 * (tilePosX / 2 % 2));
    paletteId = int(attributeFetch & uint(0x03));

    vertex.x += float(tilePosX);
    vertex.y += float(tilePosY);

    vertex.x *= meta.tileUnitWidth;
    vertex.y *= meta.tileUnitHeight;

    vertex.x = (vertex.x * 2 - 1);
    vertex.y = (vertex.y * 2 - 1) * -1;

    gl_Position = vec4(vertex, 0, 1);
}