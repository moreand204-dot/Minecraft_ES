#ifndef UI_H
#define UI_H

#include <GLES3/gl3.h>
#include <glm/glm.hpp>
#include "Shader.h"
#include "InputManager.h"
#include "Player.h"

class UI {
public:
    UI();
    ~UI();

    bool init();
    void resize(int w, int h);
    void render(InputManager& input, Player& player, float timeOfDay);

    void destroy();

private:
    Shader uiShader;
    GLuint quadVAO = 0, quadVBO = 0, quadEBO = 0;

    GLuint circleVAO = 0, circleVBO = 0;
    int circleVertexCount = 0;

    int screenWidth = 0, screenHeight = 0;

    void initQuad();
    void initCircle();

    void drawRect(float x, float y, float w, float h, float r, float g, float b, float a);
    void drawCircle(float cx, float cy, float radius, float r, float g, float b, float a, float aspect);
    void drawTexturedQuad(GLuint texture, float x, float y, float w, float h);

    void drawHotbar(InputManager& input, Player& player);
    void drawHeartsAndHunger(InputManager& input, float aspect);
    void drawTopBar(InputManager& input, float aspect, Player& player);

    GLuint createTextTexture(const char* text, int fontSize, int* outW, int* outH);
};

#endif
