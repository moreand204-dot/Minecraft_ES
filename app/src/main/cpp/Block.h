#ifndef BLOCK_H
#define BLOCK_H

#include <cstdint>

enum class BlockType : uint8_t {
    Air = 0,
    Grass = 1,
    Dirt = 2,
    Stone = 3,
    Bedrock = 4,
    Wood = 5,
    Leaves = 6,
    Sand = 7,
    Water = 8
};

struct Block {
    static bool isSolid(BlockType type);
    static bool isTransparent(BlockType type);
    static bool isLiquid(BlockType type);
    static const char* getName(BlockType type);

    // Face indices for texture atlas
    // 0=top, 1=bottom, 2=side
    static int getTextureIndex(BlockType type, int face);
};

#endif
