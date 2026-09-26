#ifndef PLAYER_H
#define PLAYER_H

#include <glm/glm.hpp>
#include "World.h"
#include "Block.h"

// Real, working inventory: 9 hotbar slots, each holding one block type + a stack count.
struct HotbarSlot {
    BlockType type = BlockType::Air;
    int count = 0;
};

enum class GameMode {
    Survival, // breaking gives you the block, placing costs one from the stack
    Creative  // unlimited blocks from a fixed palette, breaking gives nothing
};

class Player {
public:
    static const int HOTBAR_SIZE = 9;

    glm::vec3 position;
    glm::vec3 velocity;
    float yaw = -90.0f;   // degrees
    float pitch = 0.0f;   // degrees
    float width = 0.6f;
    float height = 1.8f;
    float eyeHeight = 1.62f;

    bool onGround = false;
    bool inWater = false;

    GameMode gameMode = GameMode::Survival;
    HotbarSlot hotbar[HOTBAR_SIZE];
    int selectedSlot = 0;

    Player();

    void update(float dt, World* world, const glm::vec3& moveInput, bool jump);
    void setLook(float yaw, float pitch);

    glm::vec3 getForward() const;
    glm::vec3 getRight() const;
    glm::vec3 getEyePosition() const;

    // Inventory / game mode
    void setGameMode(GameMode mode);          // resets hotbar to a sane default for that mode
    void toggleGameMode();
    BlockType getSelectedBlockType() const;    // Air if the slot is empty
    void addToInventory(BlockType type);       // survival pickup after breaking a block
    bool consumeSelected();                    // survival: try to spend 1 of the selected block; false if none left

private:
    void applyGravity(float dt);
    void applyMovement(float dt, const glm::vec3& moveInput);
    bool checkCollision(World* world, const glm::vec3& pos) const;
    void resolveCollisions(World* world, float dt);
};

#endif
