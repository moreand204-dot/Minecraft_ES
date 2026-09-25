#include "TextureAtlas.h"
#include "Logger.h"
#include <cstring>
#include <cmath>
#include <cstdlib>

TextureAtlas::TextureAtlas() {
    std::memset(pixels, 0, sizeof(pixels));
}

TextureAtlas::~TextureAtlas() {
    destroy();
}

void TextureAtlas::clearAtlas() {
    std::memset(pixels, 0, sizeof(pixels));
}

void TextureAtlas::setPixel(int x, int y, unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
    if (x < 0 || x >= ATLAS_SIZE || y < 0 || y >= ATLAS_SIZE) return;
    int idx = (y * ATLAS_SIZE + x) * 4;
    pixels[idx + 0] = r;
    pixels[idx + 1] = g;
    pixels[idx + 2] = b;
    pixels[idx + 3] = a;
}

static unsigned char randByte(int x, int y, int seed) {
    unsigned int n = (unsigned int)(x * 1619 + y * 31337 + seed * 6971);
    n = (n << 13) ^ n;
    return (unsigned char)((n * (n * n * 15731 + 789221) + 1376312589) & 0xFF);
}

void TextureAtlas::generateNoiseTile(int tileX, int tileY, unsigned char baseR, unsigned char baseG, unsigned char baseB, int variation) {
    for (int y = 0; y < TILE_SIZE; ++y) {
        for (int x = 0; x < TILE_SIZE; ++x) {
            int noise = (int)randByte(tileX * TILE_SIZE + x, tileY * TILE_SIZE + y, 7) % (variation * 2 + 1) - variation;
            int r = baseR + noise;
            int g = baseG + noise;
            int b = baseB + noise;
            if (r < 0) r = 0; if (r > 255) r = 255;
            if (g < 0) g = 0; if (g > 255) g = 255;
            if (b < 0) b = 0; if (b > 255) b = 255;
            setPixel(tileX * TILE_SIZE + x, tileY * TILE_SIZE + y, (unsigned char)r, (unsigned char)g, (unsigned char)b, 255);
        }
    }
}

void TextureAtlas::generateGrassTop(int tileX, int tileY) {
    for (int y = 0; y < TILE_SIZE; ++y) {
        for (int x = 0; x < TILE_SIZE; ++x) {
            int noise = (int)randByte(x, y, 31) % 41 - 20;
            int r = 96 + noise;
            int g = 160 + noise;
            int b = 64 + noise / 2;
            if (r < 0) r = 0; if (r > 255) r = 255;
            if (g < 0) g = 0; if (g > 255) g = 255;
            if (b < 0) b = 0; if (b > 255) b = 255;
            setPixel(tileX * TILE_SIZE + x, tileY * TILE_SIZE + y, (unsigned char)r, (unsigned char)g, (unsigned char)b, 255);
        }
    }
}

void TextureAtlas::generateGrassSide(int tileX, int tileY) {
    for (int y = 0; y < TILE_SIZE; ++y) {
        for (int x = 0; x < TILE_SIZE; ++x) {
            bool isGrass = (y < 4 + (randByte(x, 0, 99) % 3));
            if (isGrass) {
                int noise = (int)randByte(x, y, 31) % 41 - 20;
                int r = 96 + noise;
                int g = 160 + noise;
                int b = 64 + noise / 2;
                if (r < 0) r = 0; if (r > 255) r = 255;
                if (g < 0) g = 0; if (g > 255) g = 255;
                if (b < 0) b = 0; if (b > 255) b = 255;
                setPixel(tileX * TILE_SIZE + x, tileY * TILE_SIZE + y, (unsigned char)r, (unsigned char)g, (unsigned char)b, 255);
            } else {
                int noise = (int)randByte(x, y, 17) % 21 - 10;
                int r = 134 + noise;
                int g = 96 + noise;
                int b = 67 + noise;
                if (r < 0) r = 0; if (r > 255) r = 255;
                if (g < 0) g = 0; if (g > 255) g = 255;
                if (b < 0) b = 0; if (b > 255) b = 255;
                setPixel(tileX * TILE_SIZE + x, tileY * TILE_SIZE + y, (unsigned char)r, (unsigned char)g, (unsigned char)b, 255);
            }
        }
    }
}

void TextureAtlas::generateDirt(int tileX, int tileY) {
    generateNoiseTile(tileX, tileY, 134, 96, 67, 15);
}

void TextureAtlas::generateStone(int tileX, int tileY) {
    generateNoiseTile(tileX, tileY, 128, 128, 128, 18);
}

void TextureAtlas::generateBedrock(int tileX, int tileY) {
    generateNoiseTile(tileX, tileY, 60, 60, 60, 30);
}

void TextureAtlas::generateWoodTop(int tileX, int tileY) {
    for (int y = 0; y < TILE_SIZE; ++y) {
        for (int x = 0; x < TILE_SIZE; ++x) {
            float dx = x - 7.5f;
            float dy = y - 7.5f;
            float dist = std::sqrt(dx * dx + dy * dy);
            int ring = ((int)(dist * 1.5f)) & 1;
            int noise = (int)randByte(x, y, 11) % 21 - 10;
            int r = (ring ? 155 : 122) + noise;
            int g = (ring ? 110 : 85) + noise;
            int b = (ring ? 65 : 50) + noise;
            if (r < 0) r = 0; if (r > 255) r = 255;
            if (g < 0) g = 0; if (g > 255) g = 255;
            if (b < 0) b = 0; if (b > 255) b = 255;
            setPixel(tileX * TILE_SIZE + x, tileY * TILE_SIZE + y, (unsigned char)r, (unsigned char)g, (unsigned char)b, 255);
        }
    }
}

void TextureAtlas::generateWoodSide(int tileX, int tileY) {
    for (int y = 0; y < TILE_SIZE; ++y) {
        for (int x = 0; x < TILE_SIZE; ++x) {
            int noise = (int)randByte(x, y, 23) % 21 - 10;
            int streak = (int)randByte(x, 0, 41) % 21 - 10;
            int r = 110 + noise + streak / 2;
            int g = 82 + noise + streak / 2;
            int b = 48 + noise + streak / 2;
            if (r < 0) r = 0; if (r > 255) r = 255;
            if (g < 0) g = 0; if (g > 255) g = 255;
            if (b < 0) b = 0; if (b > 255) b = 255;
            setPixel(tileX * TILE_SIZE + x, tileY * TILE_SIZE + y, (unsigned char)r, (unsigned char)g, (unsigned char)b, 255);
        }
    }
}

void TextureAtlas::generateLeaves(int tileX, int tileY) {
    for (int y = 0; y < TILE_SIZE; ++y) {
        for (int x = 0; x < TILE_SIZE; ++x) {
            int n = (int)randByte(x, y, 61) % 100;
            if (n < 15) {
                setPixel(tileX * TILE_SIZE + x, tileY * TILE_SIZE + y, 0, 0, 0, 0);
            } else {
                int noise = (int)randByte(x, y, 41) % 61 - 30;
                int r = 40 + noise / 2;
                int g = 120 + noise;
                int b = 40 + noise / 2;
                if (r < 0) r = 0; if (r > 255) r = 255;
                if (g < 0) g = 0; if (g > 255) g = 255;
                if (b < 0) b = 0; if (b > 255) b = 255;
                setPixel(tileX * TILE_SIZE + x, tileY * TILE_SIZE + y, (unsigned char)r, (unsigned char)g, (unsigned char)b, 255);
            }
        }
    }
}

void TextureAtlas::generateSand(int tileX, int tileY) {
    generateNoiseTile(tileX, tileY, 220, 205, 145, 12);
}

void TextureAtlas::generateWater(int tileX, int tileY) {
    for (int y = 0; y < TILE_SIZE; ++y) {
        for (int x = 0; x < TILE_SIZE; ++x) {
            int noise = (int)randByte(x, y, 71) % 31 - 15;
            int r = 40 + noise / 2;
            int g = 90 + noise;
            int b = 180 + noise;
            if (r < 0) r = 0; if (r > 255) r = 255;
            if (g < 0) g = 0; if (g > 255) g = 255;
            if (b < 0) b = 0; if (b > 255) b = 255;
            setPixel(tileX * TILE_SIZE + x, tileY * TILE_SIZE + y, (unsigned char)r, (unsigned char)g, (unsigned char)b, 235);
        }
    }
}

bool TextureAtlas::generate() {
    clearAtlas();

    // Layout (4x4 atlas):
    // 0: grass_top  1: grass_side  2: dirt     3: stone
    // 4: bedrock    5: wood_top    6: wood_side 7: leaves
    // 8: sand       9: water

    generateGrassTop(0, 0);
    generateGrassSide(1, 0);
    generateDirt(2, 0);
    generateStone(3, 0);

    generateBedrock(0, 1);
    generateWoodTop(1, 1);
    generateWoodSide(2, 1);
    generateLeaves(3, 1);

    generateSand(0, 2);
    generateWater(1, 2);
    // 2,2 and 3,2 unused

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ATLAS_SIZE, ATLAS_SIZE, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);

    LOGI("Texture atlas generated: %dx%d", ATLAS_SIZE, ATLAS_SIZE);
    return true;
}

void TextureAtlas::bind() const {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
}

void TextureAtlas::destroy() {
    if (textureID != 0) {
        glDeleteTextures(1, &textureID);
        textureID = 0;
    }
}
