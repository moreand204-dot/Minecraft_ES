#include "Player.h"
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>

Player::Player() {
    position = glm::vec3(0.5f, 100.0f, 0.5f);
    velocity = glm::vec3(0.0f);
    setGameMode(GameMode::Survival);
}

void Player::setGameMode(GameMode mode) {
    gameMode = mode;
    for (int i = 0; i < HOTBAR_SIZE; ++i) {
        hotbar[i].type = BlockType::Air;
        hotbar[i].count = 0;
    }
    if (mode == GameMode::Creative) {
        // Fixed creative palette. Count is irrelevant in creative (never consumed)
        // but is set to 1 just so the slot renders as "occupied".
        static const BlockType palette[] = {
            BlockType::Grass, BlockType::Dirt, BlockType::Stone,
            BlockType::Wood,  BlockType::Leaves, BlockType::Sand
        };
        int n = (int)(sizeof(palette) / sizeof(palette[0]));
        for (int i = 0; i < n && i < HOTBAR_SIZE; ++i) {
            hotbar[i].type = palette[i];
            hotbar[i].count = 1;
        }
    }
    // Survival starts with an empty hotbar: you fill it by breaking blocks.
    selectedSlot = 0;
}

void Player::toggleGameMode() {
    setGameMode(gameMode == GameMode::Survival ? GameMode::Creative : GameMode::Survival);
}

BlockType Player::getSelectedBlockType() const {
    return hotbar[selectedSlot].type;
}

void Player::addToInventory(BlockType type) {
    if (gameMode == GameMode::Creative) return; // creative never needs pickups
    if (type == BlockType::Air) return;

    // 1) stack onto an existing slot of the same type (cap 64, like Minecraft)
    for (int i = 0; i < HOTBAR_SIZE; ++i) {
        if (hotbar[i].type == type && hotbar[i].count < 64) {
            hotbar[i].count++;
            return;
        }
    }
    // 2) otherwise use the first empty slot
    for (int i = 0; i < HOTBAR_SIZE; ++i) {
        if (hotbar[i].type == BlockType::Air || hotbar[i].count <= 0) {
            hotbar[i].type = type;
            hotbar[i].count = 1;
            return;
        }
    }
    // Hotbar full: block is lost (no separate backpack/inventory grid yet)
}

bool Player::consumeSelected() {
    HotbarSlot& slot = hotbar[selectedSlot];
    if (gameMode == GameMode::Creative) {
        return slot.type != BlockType::Air; // unlimited, just needs a block chosen
    }
    if (slot.type == BlockType::Air || slot.count <= 0) return false;
    slot.count--;
    if (slot.count <= 0) {
        slot.type = BlockType::Air;
        slot.count = 0;
    }
    return true;
}

glm::vec3 Player::getForward() const {
    float ry = glm::radians(yaw);
    float rp = glm::radians(pitch);
    return glm::normalize(glm::vec3(
        std::cos(ry) * std::cos(rp),
        std::sin(rp),
        std::sin(ry) * std::cos(rp)
    ));
}

glm::vec3 Player::getRight() const {
    float ry = glm::radians(yaw);
    return glm::normalize(glm::vec3(std::cos(ry + 90.0f), 0.0f, std::sin(ry + 90.0f)));
}

glm::vec3 Player::getEyePosition() const {
    return position + glm::vec3(0.0f, eyeHeight, 0.0f);
}

void Player::setLook(float y, float p) {
    yaw = y;
    pitch = glm::clamp(p, -89.0f, 89.0f);
}

bool Player::checkCollision(World* world, const glm::vec3& pos) const {
    float halfW = width * 0.5f;
    int minX = (int)std::floor(pos.x - halfW);
    int maxX = (int)std::floor(pos.x + halfW);
    int minY = (int)std::floor(pos.y);
    int maxY = (int)std::floor(pos.y + height);
    int minZ = (int)std::floor(pos.z - halfW);
    int maxZ = (int)std::floor(pos.z + halfW);

    for (int x = minX; x <= maxX; ++x) {
        for (int y = minY; y <= maxY; ++y) {
            for (int z = minZ; z <= maxZ; ++z) {
                BlockType t = world->getBlockAt(x, y, z);
                if (Block::isSolid(t)) {
                    return true;
                }
            }
        }
    }
    return false;
}

void Player::resolveCollisions(World* world, float dt) {
    (void)dt;
    // X axis
    glm::vec3 test = position;
    test.x += velocity.x * dt;
    if (checkCollision(world, test)) {
        velocity.x = 0.0f;
    } else {
        position.x = test.x;
    }

    // Z axis
    test = position;
    test.z += velocity.z * dt;
    if (checkCollision(world, test)) {
        velocity.z = 0.0f;
    } else {
        position.z = test.z;
    }

    // Y axis
    test = position;
    test.y += velocity.y * dt;
    if (checkCollision(world, test)) {
        if (velocity.y < 0) {
            onGround = true;
        }
        velocity.y = 0.0f;
    } else {
        position.y = test.y;
        if (velocity.y < -0.1f) onGround = false;
    }
}

void Player::applyGravity(float dt) {
    if (inWater) {
        velocity.y -= 9.8f * 0.3f * dt;
        velocity.y = glm::max(velocity.y, -3.0f);
    } else {
        velocity.y -= 28.0f * dt;
        if (velocity.y < -60.0f) velocity.y = -60.0f;
    }
}

void Player::applyMovement(float dt, const glm::vec3& moveInput) {
    glm::vec3 forward = getForward();
    forward.y = 0;
    if (glm::length(forward) > 0.001f) forward = glm::normalize(forward);
    glm::vec3 right = getRight();

    float speed = inWater ? 3.0f : 5.5f;
    if (!onGround && !inWater) speed = 4.5f;

    glm::vec3 wishDir = forward * moveInput.z + right * moveInput.x;
    if (glm::length(wishDir) > 0.001f) {
        wishDir = glm::normalize(wishDir);
    }

    glm::vec3 targetVel = wishDir * speed;
    float accel = onGround ? 12.0f : 4.0f;

    velocity.x += (targetVel.x - velocity.x) * accel * dt;
    velocity.z += (targetVel.z - velocity.z) * accel * dt;

    // Friction when no input
    if (glm::length(wishDir) < 0.01f && onGround) {
        velocity.x *= (1.0f - 10.0f * dt);
        velocity.z *= (1.0f - 10.0f * dt);
    }
}

void Player::update(float dt, World* world, const glm::vec3& moveInput, bool jump) {
    BlockType feetBlock = world->getBlockAt(
        (int)std::floor(position.x),
        (int)std::floor(position.y + 0.2f),
        (int)std::floor(position.z));
    BlockType eyeBlock = world->getBlockAt(
        (int)std::floor(position.x),
        (int)std::floor(position.y + eyeHeight),
        (int)std::floor(position.z));

    inWater = (feetBlock == BlockType::Water) || (eyeBlock == BlockType::Water);

    applyMovement(dt, moveInput);
    applyGravity(dt);

    if (jump) {
        if (inWater) {
            velocity.y = 3.0f;
        } else if (onGround) {
            velocity.y = 8.5f;
            onGround = false;
        }
    }

    onGround = false;
    resolveCollisions(world, dt);
}
