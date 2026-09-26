#include "UI.h"
#include "Logger.h"
#include <cmath>
#include <cstring>
#include <vector>
#include <string>
#include <android/log.h>

static const char* UI_VS = R"(
#version 300 es
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUV;
uniform mat4 uProj;
out vec2 vUV;
void main() {
    gl_Position = uProj * vec4(aPos, 0.0, 1.0);
    vUV = aUV;
}
)";

static const char* UI_FS = R"(
#version 300 es
precision mediump float;
in vec2 vUV;
uniform vec4 uColor;
uniform sampler2D uTexture;
uniform int uUseTexture;
out vec4 fragColor;
void main() {
    if (uUseTexture == 1) {
        vec4 t = texture(uTexture, vUV);
        fragColor = t * uColor;
    } else {
        fragColor = uColor;
    }
}
)";

UI::UI() {}
UI::~UI() {
    destroy();
}

bool UI::init() {
    if (!uiShader.compile(UI_VS, UI_FS)) {
        LOGE("Failed to compile UI shader");
        return false;
    }
    initQuad();
    initCircle();
    return true;
}

void UI::resize(int w, int h) {
    screenWidth = w;
    screenHeight = h;
}

void UI::destroy() {
    if (quadVAO) glDeleteVertexArrays(1, &quadVAO);
    if (quadVBO) glDeleteBuffers(1, &quadVBO);
    if (quadEBO) glDeleteBuffers(1, &quadEBO);
    if (circleVAO) glDeleteVertexArrays(1, &circleVAO);
    if (circleVBO) glDeleteBuffers(1, &circleVBO);
    uiShader.destroy();
}

void UI::initQuad() {
    float verts[] = {
        0.0f, 0.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f
    };
    uint32_t indices[] = {0, 1, 2, 0, 2, 3};

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glGenBuffers(1, &quadEBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glBindVertexArray(0);
}

void UI::initCircle() {
    const int segments = 32;
    std::vector<float> verts;
    verts.push_back(0.0f);
    verts.push_back(0.0f);
    for (int i = 0; i <= segments; ++i) {
        float a = (float)i / segments * 2.0f * 3.14159265f;
        verts.push_back(std::cos(a));
        verts.push_back(std::sin(a));
    }
    circleVertexCount = segments + 2;

    glGenVertexArrays(1, &circleVAO);
    glGenBuffers(1, &circleVBO);
    glBindVertexArray(circleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, circleVBO);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glBindVertexArray(0);
}

void UI::drawRect(float x, float y, float w, float h, float r, float g, float b, float a) {
    float aspect = (float)screenWidth / (float)screenHeight;
    float ndcX = (x / screenWidth) * 2.0f - 1.0f;
    float ndcY = 1.0f - (y / screenHeight) * 2.0f;
    float ndcW = (w / screenWidth) * 2.0f;
    float ndcH = -(h / screenHeight) * 2.0f;
    (void)aspect;

    glm::mat4 proj = glm::ortho(0.0f, (float)screenWidth, (float)screenHeight, 0.0f, -1.0f, 1.0f);
    uiShader.use();
    uiShader.setMat4("uProj", proj);
    uiShader.setVec4("uColor", glm::vec4(r, g, b, a));
    uiShader.setInt("uUseTexture", 0);
    (void)ndcX; (void)ndcY; (void)ndcW; (void)ndcH;

    // Build a temporary quad in screen coords
    float verts[] = {
        x,     y,     0.0f, 1.0f,
        x + w, y,     1.0f, 1.0f,
        x + w, y + h, 1.0f, 0.0f,
        x,     y + h, 0.0f, 0.0f
    };
    uint32_t indices[] = {0, 1, 2, 0, 2, 3};

    GLuint vao, vbo, ebo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
}

void UI::drawCircle(float cx, float cy, float radius, float r, float g, float b, float a, float aspect) {
    glm::mat4 proj = glm::ortho(0.0f, (float)screenWidth, (float)screenHeight, 0.0f, -1.0f, 1.0f);
    uiShader.use();
    uiShader.setMat4("uProj", proj);
    uiShader.setVec4("uColor", glm::vec4(r, g, b, a));
    uiShader.setInt("uUseTexture", 0);

    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(cx, cy, 0.0f));
    model = glm::scale(model, glm::vec3(radius, radius * aspect, 1.0f));
    glm::mat4 mvp = proj * model;
    uiShader.setMat4("uProj", mvp);

    glBindVertexArray(circleVAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, circleVertexCount);
    glBindVertexArray(0);
}

// Rough color swatch per block type, used to show what's in a hotbar slot
// (the project has no icon textures yet, so a flat color stands in for now).
static void blockColor(BlockType t, float& r, float& g, float& b) {
    switch (t) {
        case BlockType::Grass:  r = 0.35f; g = 0.65f; b = 0.25f; break;
        case BlockType::Dirt:   r = 0.45f; g = 0.30f; b = 0.15f; break;
        case BlockType::Stone:  r = 0.55f; g = 0.55f; b = 0.55f; break;
        case BlockType::Wood:   r = 0.50f; g = 0.35f; b = 0.15f; break;
        case BlockType::Leaves: r = 0.20f; g = 0.50f; b = 0.15f; break;
        case BlockType::Sand:   r = 0.80f; g = 0.75f; b = 0.55f; break;
        case BlockType::Bedrock:r = 0.25f; g = 0.25f; b = 0.25f; break;
        case BlockType::Water:  r = 0.20f; g = 0.35f; b = 0.85f; break;
        default:                r = 0.0f;  g = 0.0f;  b = 0.0f; break;
    }
}

void UI::drawHotbar(InputManager& input, Player& player) {
    float slot = input.hotbarSlotSize;
    float gap = input.hotbarSlotGap;
    float startX = input.hotbarOrigin.x;
    float y = input.hotbarOrigin.y;
    int n = InputManager::HOTBAR_SIZE;

    for (int i = 0; i < n; ++i) {
        float x = startX + i * (slot + gap);
        bool selected = (i == input.selectedSlot);
        if (selected) {
            // Bright outline behind the slot to mark the active one
            float pad = slot * 0.08f;
            drawRect(x - pad, y - pad, slot + pad * 2.0f, slot + pad * 2.0f, 1.0f, 1.0f, 1.0f, 0.9f);
        }
        drawRect(x, y, slot, slot, 0.0f, 0.0f, 0.0f, selected ? 0.35f : 0.25f);

        HotbarSlot& item = player.hotbar[i];
        if (item.type != BlockType::Air) {
            float r, g, b;
            blockColor(item.type, r, g, b);
            float pad = slot * 0.16f;
            drawRect(x + pad, y + pad, slot - pad * 2.0f, slot - pad * 2.0f, r, g, b, 0.95f);

            // Stack count shown as a small fill bar along the bottom edge of the slot
            // instead of a number (no font/text rendering in the project yet).
            if (player.gameMode == GameMode::Survival) {
                float frac = glm::clamp(item.count / 64.0f, 0.05f, 1.0f);
                float barH = slot * 0.08f;
                drawRect(x + pad, y + slot - pad - barH, (slot - pad * 2.0f) * frac, barH, 1.0f, 1.0f, 1.0f, 0.85f);
            }
        }
    }
}

void UI::drawHeartsAndHunger(InputManager& input, float aspect) {
    float slot = input.hotbarSlotSize;
    float pipR = slot * 0.16f;
    float pipSpacing = slot * 0.38f;
    float rowY = input.hotbarOrigin.y - slot * 0.75f;

    // Hearts: left-aligned above the hotbar (health is not simulated yet, so shown full)
    float heartsStartX = input.hotbarOrigin.x + pipR;
    for (int i = 0; i < 10; ++i) {
        float cx = heartsStartX + i * pipSpacing;
        drawCircle(cx, rowY, pipR * 1.25f, 0.0f, 0.0f, 0.0f, 0.35f, aspect); // outline
        drawCircle(cx, rowY, pipR, 0.82f, 0.1f, 0.15f, 0.95f, aspect);
    }

    // Hunger: right-aligned above the hotbar (not simulated yet, shown full)
    float hotbarWidth = InputManager::HOTBAR_SIZE * slot + (InputManager::HOTBAR_SIZE - 1) * input.hotbarSlotGap;
    float hungerEndX = input.hotbarOrigin.x + hotbarWidth - pipR;
    for (int i = 0; i < 10; ++i) {
        float cx = hungerEndX - i * pipSpacing;
        drawCircle(cx, rowY, pipR * 1.25f, 0.0f, 0.0f, 0.0f, 0.35f, aspect); // outline
        drawCircle(cx, rowY, pipR, 0.75f, 0.45f, 0.15f, 0.95f, aspect);
    }
}

void UI::drawTopBar(InputManager& input, float aspect, Player& player) {
    float r = input.topBtnRadius;

    // Pause / menu -- currently doubles as the Creative/Survival toggle
    // (placeholder control until a real pause menu screen exists)
    glm::vec2 c = input.topBtnPause;
    drawCircle(c.x, c.y, r, 0.0f, 0.0f, 0.0f, 0.35f, aspect);
    if (player.gameMode == GameMode::Creative) {
        drawCircle(c.x, c.y, r * 0.55f, 0.3f, 0.85f, 0.3f, 0.9f, aspect); // green dot = Creative
    } else {
        drawRect(c.x - r * 0.35f, c.y - r * 0.45f, r * 0.22f, r * 0.9f, 1.0f, 1.0f, 1.0f, 0.9f);
        drawRect(c.x + r * 0.1f, c.y - r * 0.45f, r * 0.22f, r * 0.9f, 1.0f, 1.0f, 1.0f, 0.9f);
    }

    // Chat (not implemented yet - placeholder)
    c = input.topBtnChat;
    drawCircle(c.x, c.y, r, 0.0f, 0.0f, 0.0f, 0.35f, aspect);
    drawRect(c.x - r * 0.5f, c.y - r * 0.3f, r * 1.0f, r * 0.6f, 1.0f, 1.0f, 1.0f, 0.9f);

    // View / perspective toggle (not implemented yet - placeholder)
    c = input.topBtnView;
    drawCircle(c.x, c.y, r, 0.0f, 0.0f, 0.0f, 0.35f, aspect);
    drawCircle(c.x, c.y, r * 0.5f, 1.0f, 1.0f, 1.0f, 0.9f, aspect);
}

void UI::render(InputManager& input, Player& player, float timeOfDay) {
    (void)timeOfDay;
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float aspect = (float)screenWidth / (float)screenHeight;

    // Joystick base
    float jx = input.joystickCenter.x;
    float jy = input.joystickCenter.y;
    float jr = input.joystickRadius;

    drawCircle(jx, jy, jr, 1.0f, 1.0f, 1.0f, 0.25f, aspect);
    drawCircle(jx, jy, jr * 0.85f, 0.0f, 0.0f, 0.0f, 0.15f, aspect);

    // Joystick knob
    float knobX = jx + input.joystickDelta.x * jr;
    float knobY = jy + input.joystickDelta.y * jr;
    drawCircle(knobX, knobY, jr * 0.35f, 1.0f, 1.0f, 1.0f, 0.7f, aspect);

    // Jump button
    float bx = input.jumpButtonCenter.x;
    float by = input.jumpButtonCenter.y;
    float br = input.jumpButtonRadius;
    drawCircle(bx, by, br, 1.0f, 1.0f, 1.0f, 0.3f, aspect);
    drawCircle(bx, by, br * 0.75f, 0.4f, 0.9f, 0.4f, 0.6f, aspect);

    // HUD: hotbar + hearts/hunger + top bar (matches the reference layout)
    drawHotbar(input, player);
    drawHeartsAndHunger(input, aspect);
    drawTopBar(input, aspect, player);

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
}
