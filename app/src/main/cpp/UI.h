#ifndef UI_H
#define UI_H

#include <GLES3/gl3.h>
#include <glm/glm.hpp>
#include "Shader.h"
#include "InputManager.h"

class UI {
public:
    UI();
    ~UI();

    bool init();
    void resize(int w, int h);
    void render(InputManager& input, const glm::vec3& playerPos, float timeOfDay);

    void destroy();

private:
    Shader uiShader;
    GLuint quadVAO = 0, quadVBO = 0, quadEBO = 0;

    GLuint circleVAO = 0, circleVBO = 0;
    int circleVertexCount = 0;

    int screenWidth = 0, screenHeight = 0;

    // Hotbar layout, computed each frame by drawHotbar() and reused by drawHeartsAndHunger()
    float hotbarX = 0.0f, hotbarY = 0.0f, hotbarSlot = 0.0f, hotbarGap = 0.0f;

    void initQuad();
    void initCircle();

    void drawRect(float x, float y, float w, float h, float r, float g, float b, float a);
    void drawCircle(float cx, float cy, float radius, float r, float g, float b, float a, float aspect);
    void drawTexturedQuad(GLuint texture, float x, float y, float w, float h);

    void drawHotbar(InputManager& input);
    void drawHeartsAndHunger(float aspect);
    void drawTopBar(float aspect);

    GLuint createTextTexture(const char* text, int fontSize, int* outW, int* outH);
};

#endif
