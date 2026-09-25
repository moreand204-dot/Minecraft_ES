#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include <glm/glm.hpp>

class InputManager {
public:
    // Movement
    glm::vec2 joystickDelta = glm::vec2(0.0f);
    bool jumpPressed = false;

    // Look
    glm::vec2 lookDelta = glm::vec2(0.0f);
    bool lookActive = false;

    // Block interaction
    bool breakPressed = false; // long press
    bool placePressed = false; // short tap
    float tapX = 0.0f, tapY = 0.0f;

    // Joystick center for rendering
    glm::vec2 joystickCenter = glm::vec2(-1.0f, -1.0f);
    glm::vec2 jumpButtonCenter = glm::vec2(-1.0f, -1.0f);
    float joystickRadius = 0.0f;
    float jumpButtonRadius = 0.0f;

    // Screen size
    int screenWidth = 0;
    int screenHeight = 0;

    void setScreenSize(int w, int h);

    // Called from native input events
    void onTouchDown(int id, float x, float y);
    void onTouchMove(int id, float x, float y);
    void onTouchUp(int id, float x, float y);

    // Must be called once per frame (before reading breakPressed) so long-press
    // mining can be detected even while the finger stays perfectly still.
    void tick(float dt);

    void reset();

    // Hotbar (visual selection only)
    int selectedSlot = 0;
    static const int HOTBAR_SIZE = 9;

private:
    static const int MAX_POINTERS = 10;
    static constexpr float HOLD_THRESHOLD = 0.35f;  // seconds to trigger mining
    static constexpr float MOVE_THRESHOLD = 25.0f;  // px of drift still counted as "held still"
    struct PointerState {
        bool active = false;
        float x = 0.0f, y = 0.0f;
        float startX = 0.0f, startY = 0.0f;
        int role = 0; // 0=none, 1=joystick, 2=look, 3=jump, 4=place/break
        float startTime = 0.0f;
        float holdElapsed = 0.0f;
        bool longPressTriggered = false;
    };
    PointerState pointers[MAX_POINTERS];
};

#endif
