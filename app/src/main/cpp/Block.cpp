#include "Block.h"

bool Block::isSolid(BlockType type) {
    switch (type) {
        case BlockType::Air:
        case BlockType::Water:
            return false;
        default:
            return true;
    }
}

bool Block::isTransparent(BlockType type) {
    switch (type) {
        case BlockType::Air:
        case BlockType::Water:
        case BlockType::Leaves:
            return true;
        default:
            return false;
    }
}

bool Block::isLiquid(BlockType type) {
    return type == BlockType::Water;
}

const char* Block::getName(BlockType type) {
    switch (type) {
        case BlockType::Air: return "Air";
        case BlockType::Grass: return "Grass";
        case BlockType::Dirt: return "Dirt";
        case BlockType::Stone: return "Stone";
        case BlockType::Bedrock: return "Bedrock";
        case BlockType::Wood: return "Wood";
        case BlockType::Leaves: return "Leaves";
        case BlockType::Sand: return "Sand";
        case BlockType::Water: return "Water";
    }
    return "Unknown";
}

int Block::getTextureIndex(BlockType type, int face) {
    // Atlas layout: 4 columns x 4 rows = 16 tiles
    // 0: grass_top, 1: grass_side, 2: dirt, 3: stone
    // 4: bedrock, 5: wood_top, 6: wood_side, 7: leaves
    // 8: sand, 9: water
    switch (type) {
        case BlockType::Grass:
            if (face == 0) return 0; // top
            if (face == 1) return 2; // bottom = dirt
            return 1; // side
        case BlockType::Dirt:
            return 2;
        case BlockType::Stone:
            return 3;
        case BlockType::Bedrock:
            return 4;
        case BlockType::Wood:
            if (face == 0 || face == 1) return 5; // top/bottom
            return 6; // side
        case BlockType::Leaves:
            return 7;
        case BlockType::Sand:
            return 8;
        case BlockType::Water:
            return 9;
        default:
            return 0;
    }
}
