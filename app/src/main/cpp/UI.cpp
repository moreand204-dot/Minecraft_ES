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

void UI::render(InputManager& input, const glm::vec3& playerPos, float timeOfDay) {
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

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    (void)playerPos;
}
