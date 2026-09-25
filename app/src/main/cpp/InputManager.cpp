#include "InputManager.h"
#include <cmath>
#include <android/log.h>

void InputManager::setScreenSize(int w, int h) {
    screenWidth = w;
    screenHeight = h;

    // Place joystick bottom-left
    float margin = h * 0.12f;
    joystickRadius = h * 0.14f;
    joystickCenter = glm::vec2(margin + joystickRadius, h - margin - joystickRadius);

    // Jump button bottom-right
    jumpButtonRadius = h * 0.08f;
    jumpButtonCenter = glm::vec2(w - margin - jumpButtonRadius * 2.5f, h - margin - jumpButtonRadius);
}

void InputManager::reset() {
    joystickDelta = glm::vec2(0.0f);
    lookDelta = glm::vec2(0.0f);
    jumpPressed = false;
    breakPressed = false;
    placePressed = false;
    lookActive = false;
}

void InputManager::onTouchDown(int id, float x, float y) {
    if (id < 0 || id >= MAX_POINTERS) return;
    PointerState& p = pointers[id];
    p.active = true;
    p.x = x;
    p.y = y;
    p.startX = x;
    p.startY = y;
    p.holdElapsed = 0.0f;
    p.longPressTriggered = false;

    // Determine role based on position
    float dx = x - joystickCenter.x;
    float dy = y - joystickCenter.y;
    float distJoy = std::sqrt(dx * dx + dy * dy);

    float jdx = x - jumpButtonCenter.x;
    float jdy = y - jumpButtonCenter.y;
    float distJump = std::sqrt(jdx * jdx + jdy * jdy);

    if (distJoy < joystickRadius * 1.6f) {
        p.role = 1; // joystick
    } else if (distJump < jumpButtonRadius * 1.8f) {
        p.role = 3; // jump
        jumpPressed = true;
    } else {
        p.role = 2; // look / camera
        lookActive = true;
    }
}

void InputManager::onTouchMove(int id, float x, float y) {
    if (id < 0 || id >= MAX_POINTERS) return;
    PointerState& p = pointers[id];
    if (!p.active) return;

    if (p.role == 1) {
        float dx = x - joystickCenter.x;
        float dy = y - joystickCenter.y;
        float dist = std::sqrt(dx * dx + dy * dy);
        if (dist > joystickRadius) {
            dx = dx / dist * joystickRadius;
            dy = dy / dist * joystickRadius;
        }
        joystickDelta = glm::vec2(dx / joystickRadius, dy / joystickRadius);
    } else if (p.role == 2) {
        // Once a long-press (mining) has been recognized on this finger, freeze the
        // camera so small tremors while mining don't spin the view.
        if (!p.longPressTriggered) {
            float dx = x - p.x;
            float dy = y - p.y;
            lookDelta += glm::vec2(dx, dy);
        }
    }

    p.x = x;
    p.y = y;
}

void InputManager::tick(float dt) {
    for (int i = 0; i < MAX_POINTERS; ++i) {
        PointerState& p = pointers[i];
        if (!p.active || p.role != 2 || p.longPressTriggered) continue;

        float dx = p.x - p.startX;
        float dy = p.y - p.startY;
        float moveDist = std::sqrt(dx * dx + dy * dy);

        if (moveDist > MOVE_THRESHOLD) {
            // Finger is dragging (looking around), not holding still on a block.
            // Reset the timer so a subsequent still-hold can still mine.
            p.holdElapsed = 0.0f;
            continue;
        }

        p.holdElapsed += dt;
        if (p.holdElapsed >= HOLD_THRESHOLD) {
            p.longPressTriggered = true;
            breakPressed = true; // consumed by Engine::update() this frame
        }
    }
}

void InputManager::onTouchUp(int id, float x, float y) {
    if (id < 0 || id >= MAX_POINTERS) return;
    PointerState& p = pointers[id];
    if (!p.active) return;

    if (p.role == 1) {
        joystickDelta = glm::vec2(0.0f);
    } else if (p.role == 2) {
        float dx = x - p.startX;
        float dy = y - p.startY;
        float moveDist = std::sqrt(dx * dx + dy * dy);
        // Only a quick, short tap places a block. If this finger already triggered
        // mining (long press), releasing it must NOT also place a block.
        if (moveDist < 20.0f && !p.longPressTriggered) {
            placePressed = true;
            tapX = x;
            tapY = y;
        }
    } else if (p.role == 3) {
        jumpPressed = false;
    }

    p.active = false;
    p.role = 0;

    // Check if any look pointer still active
    lookActive = false;
    for (int i = 0; i < MAX_POINTERS; ++i) {
        if (pointers[i].active && pointers[i].role == 2) {
            lookActive = true;
            break;
        }
    }
}
