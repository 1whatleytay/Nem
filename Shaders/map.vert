#version 330 core

layout (location = 0) in vec2 pos;

flat out int palette;
out vec2 texCoord;

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

float space(float pos) { return (pos * 2 - 1); }

void main() {
    int shapeId = gl_VertexID / 6;
    int vertexType = gl_VertexID % 6;

    int tilePosX = shapeId % meta.mapWidth, tilePosY = shapeId / meta.mapWidth;

    vec2 vertex = vertexTypes[vertexType];

    vertex.x += float(tilePosX);
    vertex.y += float(tilePosY);

    vertex.x *= meta.tileUnitWidth;
    vertex.y *= meta.tileUnitHeight;

    vertex.x = space(vertex.x);
    vertex.y = space(vertex.y) * -1;

    palette = shapeId;

    gl_Position = vec4(vertex, 0, 1);
}