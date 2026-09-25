#ifndef RENDERER_H
#define RENDERER_H

#include <GLES3/gl3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Shader.h"
#include "World.h"
#include "Player.h"
#include "TextureAtlas.h"

class Renderer {
public:
    Renderer();
    ~Renderer();

    bool init();
    void resize(int width, int height);
    void render(World* world, Player* player, float timeOfDay, const World::RayHit& hit, bool hasHit);

    glm::mat4 getProjection() const { return projection; }
    glm::mat4 getView(Player* player) const;

    TextureAtlas* getAtlas() { return &atlas; }

    GLuint highlightVAO = 0, highlightVBO = 0, highlightEBO = 0;
    void renderHighlight(const World::RayHit& hit);

private:
    Shader blockShader;
    Shader lineShader;
    TextureAtlas atlas;

    glm::mat4 projection;
    glm::mat4 currentVP;

    int screenWidth = 0;
    int screenHeight = 0;

    GLuint crosshairVAO = 0, crosshairVBO = 0;
    int crosshairVertexCount = 0;

    void initCrosshair();
    void renderCrosshair();
    void renderSky(float timeOfDay);
    void initHighlight();
};

#endif
