#ifndef PLAYER_H
#define PLAYER_H

#include <glm/glm.hpp>
#include "World.h"

class Player {
public:
    glm::vec3 position;
    glm::vec3 velocity;
    float yaw = -90.0f;   // degrees
    float pitch = 0.0f;   // degrees
    float width = 0.6f;
    float height = 1.8f;
    float eyeHeight = 1.62f;

    bool onGround = false;
    bool inWater = false;

    Player();

    void update(float dt, World* world, const glm::vec3& moveInput, bool jump);
    void setLook(float yaw, float pitch);

    glm::vec3 getForward() const;
    glm::vec3 getRight() const;
    glm::vec3 getEyePosition() const;

private:
    void applyGravity(float dt);
    void applyMovement(float dt, const glm::vec3& moveInput);
    bool checkCollision(World* world, const glm::vec3& pos) const;
    void resolveCollisions(World* world, float dt);
};

#endif
