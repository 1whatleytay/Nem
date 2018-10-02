#version 330 core

flat in int palette;
in vec2 texCoord;
out vec4 color;

const float c = 255.0f;

vec3 palette2C02[] = vec3[](
            vec3(84.0f/c, 84.0f/c, 84.0f/c ), // PL 0x00
            vec3(0.0f/c, 30.0f/c, 116.0f/c ), // PL 0x01
            vec3(8.0f/c, 16.0f/c, 155.0f/c ), // PL 0x02
            vec3(48.0f/c, 0.0f/c, 136.0f/c ), // PL 0x03
            vec3(68.0f/c, 0.0f/c, 100.0f/c ), // PL 0x04
            vec3(92.0f/c, 0.0f/c, 48.0f/c ), // PL 0x05
            vec3(84.0f/c, 4.0f/c, 0.0f/c ), // PL 0x06
            vec3(60.0f/c, 24.0f/c, 0.0f/c ), // PL 0x07
            vec3(32.0f/c, 42.0f/c, 0.0f/c ), // PL 0x08
            vec3(8.0f/c, 58.0f/c, 0.0f/c ), // PL 0x09
            vec3(0.0f/c, 64.0f/c, 0.0f/c ), // PL 0x0A
            vec3(0.0f/c, 60.0f/c, 0.0f/c ), // PL 0x0B
            vec3(0.0f/c, 50.0f/c, 60.0f/c ), // PL 0x0C
            vec3(0.0f/c, 0.0f/c, 0.0f/c ), // PL 0x0D
            vec3(0.0f/c, 0.0f/c, 0.0f/c ), // PL 0x0E
            vec3(0.0f/c, 0.0f/c, 0.0f/c ), // PL 0x0F
            vec3(152.0f/c, 150.0f/c, 152.0f/c), // PL 0x10
            vec3(8.0f/c, 76.0f/c, 196.0f/c), // PL 0x11
            vec3(48.0f/c, 50.0f/c, 236.0f/c), // PL 0x12
            vec3(92.0f/c, 30.0f/c, 228.0f/c), // PL 0x13
            vec3(136.0f/c, 20.0f/c, 176.0f/c), // PL 0x14
            vec3(160.0f/c, 20.0f/c, 100.0f/c), // PL 0x15
            vec3(152.0f/c, 34.0f/c, 32.0f/c), // PL 0x16
            vec3(120.0f/c, 60.0f/c, 0.0f/c), // PL 0x17
            vec3(84.0f/c, 90.0f/c, 0.0f/c), // PL 0x18
            vec3(40.0f/c, 114.0f/c, 0.0f/c), // PL 0x19
            vec3(8.0f/c, 124.0f/c, 0.0f/c), // PL 0x1A
            vec3(0.0f/c, 118.0f/c, 40.0f/c), // PL 0x1B
            vec3(0.0f/c, 102.0f/c, 120.0f/c), // PL 0x1C
            vec3(0.0f/c, 0.0f/c, 0.0f/c), // PL 0x1D
            vec3(0.0f/c, 0.0f/c, 0.0f/c), // PL 0x1E
            vec3(0.0f/c, 0.0f/c, 0.0f/c), // PL 0x1F
            vec3(236.0f/c, 238.0f/c, 236.0f/c), // PL 0x20
            vec3(76.0f/c, 154.0f/c, 236.0f/c), // PL 0x21
            vec3(120.0f/c, 124.0f/c, 236.0f/c), // PL 0x22
            vec3(176.0f/c, 98.0f/c, 236.0f/c), // PL 0x23
            vec3(228.0f/c, 84.0f/c, 236.0f/c), // PL 0x24
            vec3(236.0f/c, 88.0f/c, 180.0f/c), // PL 0x25
            vec3(236.0f/c, 106.0f/c, 100.0f/c), // PL 0x26
            vec3(212.0f/c, 136.0f/c, 32.0f/c), // PL 0x27
            vec3(160.0f/c, 170.0f/c, 0.0f/c), // PL 0x28
            vec3(116.0f/c, 196.0f/c, 0.0f/c), // PL 0x29
            vec3(76.0f/c, 208.0f/c, 32.0f/c), // PL 0x2A
            vec3(56.0f/c, 204.0f/c, 108.0f/c), // PL 0x2B
            vec3(56.0f/c, 180.0f/c, 204.0f/c), // PL 0x2C
            vec3(60.0f/c, 60.0f/c, 60.0f/c), // PL 0x2D
            vec3(0.0f/c, 0.0f/c, 0.0f/c), // PL 0x2E
            vec3(0.0f/c, 0.0f/c, 0.0f/c), // PL 0x2F
            vec3(236.0f/c, 238.0f/c, 236.0f/c), // PL 0x30
            vec3(168.0f/c, 204.0f/c, 236.0f/c), // PL 0x31
            vec3(188.0f/c, 188.0f/c, 236.0f/c), // PL 0x32
            vec3(212.0f/c, 178.0f/c, 236.0f/c), // PL 0x33
            vec3(236.0f/c, 174.0f/c, 236.0f/c), // PL 0x34
            vec3(236.0f/c, 174.0f/c, 212.0f/c), // PL 0x35
            vec3(236.0f/c, 180.0f/c, 176.0f/c), // PL 0x36
            vec3(228.0f/c, 196.0f/c, 144.0f/c), // PL 0x37
            vec3(204.0f/c, 210.0f/c, 120.0f/c), // PL 0x38
            vec3(180.0f/c, 222.0f/c, 120.0f/c), // PL 0x39
            vec3(168.0f/c, 226.0f/c, 144.0f/c), // PL 0x3A
            vec3(152.0f/c, 226.0f/c, 180.0f/c), // PL 0x3B
            vec3(160.0f/c, 214.0f/c, 228.0f/c), // PL 0x3C
            vec3(160.0f/c, 162.0f/c, 160.0f/c), // PL 0x3D
            vec3(0.0f/c, 0.0f/c, 0.0f/c), // PL 0x3E
            vec3(0.0f/c, 0.0f/c, 0.0f/c) // PL 0x3F
    );

void main() {
    color = vec4(palette2C02[palette % 0x40], 1);
}
