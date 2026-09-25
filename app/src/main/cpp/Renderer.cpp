#include "Renderer.h"
#include "Logger.h"
#include <vector>

static const char* BLOCK_VS = R"(
#version 300 es
layout(location=0) in vec3 aPos;
layout(location=1) in vec2 aUV;
layout(location=2) in vec3 aNormal;
layout(location=3) in float aAO;
uniform mat4 uMVP;
out vec2 vUV;
out float vLight;
void main(){
 gl_Position = uMVP * vec4(aPos,1.0);
 vUV = aUV;
 float ndl = max(dot(normalize(aNormal), normalize(vec3(0.4,1.0,0.3))),0.0);
 vLight = (0.5+0.5*ndl)*aAO;
}
)";
static const char* BLOCK_FS = R"(
#version 300 es
precision mediump float;
in vec2 vUV; in float vLight;
uniform sampler2D uTexture;
uniform float uAmbient;
out vec4 fragColor;
void main(){
 vec4 tex = texture(uTexture, vUV);
 if(tex.a < 0.1) discard;
 fragColor = vec4(tex.rgb * vLight * uAmbient, tex.a);
}
)";
static const char* LINE_VS = R"(
#version 300 es
layout(location=0) in vec3 aPos;
uniform mat4 uMVP;
void main(){ gl_Position = uMVP * vec4(aPos,1.0); }
)";
static const char* LINE_FS = R"(
#version 300 es
precision mediump float;
uniform vec4 uColor;
out vec4 fragColor;
void main(){ fragColor = uColor; }
)";

Renderer::Renderer(){}
Renderer::~Renderer(){
 if(crosshairVAO) glDeleteVertexArrays(1,&crosshairVAO);
 if(crosshairVBO) glDeleteBuffers(1,&crosshairVBO);
 if(highlightVAO) glDeleteVertexArrays(1,&highlightVAO);
 if(highlightVBO) glDeleteBuffers(1,&highlightVBO);
 if(highlightEBO) glDeleteBuffers(1,&highlightEBO);
 atlas.destroy();
 blockShader.destroy();
 lineShader.destroy();
}

void Renderer::initCrosshair(){
 if(crosshairVAO==0) glGenVertexArrays(1,&crosshairVAO);
 if(crosshairVBO==0) glGenBuffers(1,&crosshairVBO);
 float verts[] = {-15,0,0, 15,0,0, 0,-15,0, 0,15,0};
 glBindVertexArray(crosshairVAO);
 glBindBuffer(GL_ARRAY_BUFFER, crosshairVBO);
 glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
 glEnableVertexAttribArray(0);
 glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,0);
 glBindVertexArray(0);
}
void Renderer::initHighlight(){
 if(highlightVAO==0) glGenVertexArrays(1,&highlightVAO);
 if(highlightVBO==0) glGenBuffers(1,&highlightVBO);
 if(highlightEBO==0) glGenBuffers(1,&highlightEBO);
 float v[] = {0,0,0, 1,0,0, 1,1,0, 0,1,0, 0,0,1, 1,0,1, 1,1,1, 0,1,1};
 unsigned int idx[] = {0,1,1,2,2,3,3,0, 4,5,5,6,6,7,7,4, 0,4,1,5,2,6,3,7};
 glBindVertexArray(highlightVAO);
 glBindBuffer(GL_ARRAY_BUFFER, highlightVBO);
 glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW);
 glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, highlightEBO);
 glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);
 glEnableVertexAttribArray(0);
 glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,0);
 glBindVertexArray(0);
}
void Renderer::renderCrosshair(){
 if(crosshairVAO==0) return;
 glDisable(GL_DEPTH_TEST);
 glm::mat4 proj = glm::ortho(0.0f,(float)screenWidth,(float)screenHeight,0.0f,-1.0f,1.0f);
 glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(screenWidth/2.0f, screenHeight/2.0f, 0.0f));
 lineShader.use();
 lineShader.setMat4("uMVP", proj*model);
 lineShader.setVec4("uColor", glm::vec4(1,1,1,1));
 glBindVertexArray(crosshairVAO);
 glDrawArrays(GL_LINES,0,4);
 glBindVertexArray(0);
 glEnable(GL_DEPTH_TEST);
}
void Renderer::renderHighlight(const World::RayHit& hit){
 if(highlightVAO==0) return;
 glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3((float)hit.x,(float)hit.y,(float)hit.z));
 glm::vec3(1.001f) * 0.0f;
 model = glm::translate(glm::mat4(1.0f), glm::vec3(hit.x,hit.y,hit.z));
 model = glm::scale(model, glm::vec3(1.001f));
 lineShader.use();
 lineShader.setMat4("uMVP", currentVP*model);
 lineShader.setVec4("uColor", glm::vec4(0,0,0,1));
 glBindVertexArray(highlightVAO);
 glDrawElements(GL_LINES,24,GL_UNSIGNED_INT,0);
 glBindVertexArray(0);
}

bool Renderer::init(){
 if(!blockShader.compile(BLOCK_VS,BLOCK_FS)){ LOGE("block shader fail"); return false; }
 if(!lineShader.compile(LINE_VS,LINE_FS)){ LOGE("line shader fail"); return false; }
 if(!atlas.generate()){ LOGE("atlas fail"); return false; }
 initCrosshair();
 initHighlight();
 glEnable(GL_DEPTH_TEST);
 glEnable(GL_CULL_FACE);
 glCullFace(GL_BACK);
 glFrontFace(GL_CCW);
 glEnable(GL_BLEND);
 glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
 LOGI("Renderer initialized");
 return true;
}
void Renderer::resize(int w,int h){
 if(w<=0||h<=0) return;
 screenWidth=w; screenHeight=h;
 glViewport(0,0,w,h);
 float aspect=(float)w/(float)h;
 if(aspect<=0) aspect=1.0f;
 projection=glm::perspective(glm::radians(70.0f),aspect,0.1f,500.0f);
}
glm::mat4 Renderer::getView(Player* p) const {
 glm::vec3 eye=p->getEyePosition();
 return glm::lookAt(eye, eye+p->getForward(), glm::vec3(0,1,0));
}
void Renderer::renderSky(float timeOfDay){
 float sunAngle=timeOfDay*2.0f*3.14159265f;
 float b=0.5f+0.5f*std::sin(sunAngle);
 float r,g,bl;
 if(b>0.7f){ r=0.4f; g=0.6f; bl=0.95f; }
 else if(b>0.4f){ float t=(b-0.4f)/0.3f; r=0.9f*(1-t)+0.4f*t; g=0.5f*(1-t)+0.6f*t; bl=0.3f*(1-t)+0.95f*t; }
 else { float t=b/0.4f; r=0.05f*(1-t)+0.9f*t; g=0.05f*(1-t)+0.5f*t; bl=0.15f*(1-t)+0.3f*t; }
 glClearColor(r,g,bl,1.0f);
 glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
}
void Renderer::render(World* world, Player* player, float timeOfDay, const World::RayHit& hit, bool hasHit){
 if(!world||!player) return;
 renderSky(timeOfDay);
 glm::mat4 view=getView(player);
 glm::mat4 vp=projection*view;
 currentVP=vp;
 float sunAngle=timeOfDay*2.0f*3.14159265f;
 float ambient=0.3f+0.7f*glm::clamp(std::sin(sunAngle)*1.5f+0.3f,0.0f,1.0f);
 blockShader.use();
 blockShader.setMat4("uMVP",vp);
 blockShader.setFloat("uAmbient",ambient);
 blockShader.setInt("uTexture",0);
 atlas.bind();
 world->render();
 if(hasHit) renderHighlight(hit);
 renderCrosshair();
}
