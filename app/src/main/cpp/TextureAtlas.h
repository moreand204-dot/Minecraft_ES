#ifndef TEXTURE_ATLAS_H
#define TEXTURE_ATLAS_H

#include <GLES3/gl3.h>

class TextureAtlas {
public:
    GLuint textureID = 0;

    TextureAtlas();
    ~TextureAtlas();

    bool generate();
    void bind() const;
    void destroy();

private:
    static const int TILE_SIZE = 16;
    static const int ATLAS_COLS = 4;
    static const int ATLAS_ROWS = 4;
    static const int ATLAS_SIZE = TILE_SIZE * ATLAS_COLS;

    unsigned char pixels[ATLAS_SIZE * ATLAS_SIZE * 4];

    void clearAtlas();
    void setPixel(int x, int y, unsigned char r, unsigned char g, unsigned char b, unsigned char a);
    void generateNoiseTile(int tileX, int tileY, unsigned char baseR, unsigned char baseG, unsigned char baseB, int variation);
    void generateGrassTop(int tileX, int tileY);
    void generateGrassSide(int tileX, int tileY);
    void generateDirt(int tileX, int tileY);
    void generateStone(int tileX, int tileY);
    void generateBedrock(int tileX, int tileY);
    void generateWoodTop(int tileX, int tileY);
    void generateWoodSide(int tileX, int tileY);
    void generateLeaves(int tileX, int tileY);
    void generateSand(int tileX, int tileY);
    void generateWater(int tileX, int tileY);
    void drawBreakOverlay(int tileIndex, float progress);
};

#endif
