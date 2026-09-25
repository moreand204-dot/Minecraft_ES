#include <android_native_app_glue.h>
#include <android/input.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>
#include <cmath>
#include <memory>
#include <string>

#include "Logger.h"
#include "Renderer.h"
#include "World.h"
#include "Player.h"
#include "InputManager.h"
#include "UI.h"
#include "SaveManager.h"

struct Engine {
    android_app* app = nullptr;

    EGLDisplay display = EGL_NO_DISPLAY;
    EGLSurface surface = EGL_NO_SURFACE;
    EGLContext context = EGL_NO_CONTEXT;
    EGLConfig config = nullptr;

    bool hasSurface = false;
    bool hasFocus = false;
    bool running = false;

    int width = 0, height = 0;

    std::unique_ptr<Renderer> renderer;
    std::unique_ptr<World> world;
    std::unique_ptr<Player> player;
    std::unique_ptr<UI> ui;

    InputManager input;

    float timeOfDay = 0.0f; // 0..1
    float dayLength = 600.0f; // seconds for full cycle

    std::chrono::steady_clock::time_point lastTime;
    float accumulator = 0.0f;
    float saveTimer = 0.0f;
    bool initialized = false;

    std::string savePath;

    void init();
    void shutdown();
    void createSurface();
    void destroySurface();
    void handleInput();
    void update(float dt);
    void render();
    void frame();
    void save();
};

static Engine gEngine;

static int32_t handleInput(struct android_app* app, AInputEvent* event) {
    Engine* e = (Engine*)app->userData;
    if (!e) return 0;

    int32_t type = AInputEvent_getType(event);
    if (type == AINPUT_EVENT_TYPE_MOTION) {
        int32_t action = AMotionEvent_getAction(event);
        int32_t actionMasked = action & AMOTION_EVENT_ACTION_MASK;
        size_t pointerIndex = (action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;

        if (actionMasked == AMOTION_EVENT_ACTION_DOWN || actionMasked == AMOTION_EVENT_ACTION_POINTER_DOWN) {
            int32_t id = AMotionEvent_getPointerId(event, pointerIndex);
            float x = AMotionEvent_getX(event, pointerIndex);
            float y = AMotionEvent_getY(event, pointerIndex);
            e->input.onTouchDown(id, x, y);
            return 1;
        } else if (actionMasked == AMOTION_EVENT_ACTION_MOVE) {
            size_t count = AMotionEvent_getPointerCount(event);
            for (size_t i = 0; i < count; ++i) {
                int32_t id = AMotionEvent_getPointerId(event, i);
                float x = AMotionEvent_getX(event, i);
                float y = AMotionEvent_getY(event, i);
                e->input.onTouchMove(id, x, y);
            }
            return 1;
        } else if (actionMasked == AMOTION_EVENT_ACTION_UP || actionMasked == AMOTION_EVENT_ACTION_POINTER_UP) {
            int32_t id = AMotionEvent_getPointerId(event, pointerIndex);
            float x = AMotionEvent_getX(event, pointerIndex);
            float y = AMotionEvent_getY(event, pointerIndex);
            e->input.onTouchUp(id, x, y);
            return 1;
        } else if (actionMasked == AMOTION_EVENT_ACTION_CANCEL) {
            e->input.reset();
            return 1;
        }
    }

    return 0;
}

static void handleCmd(struct android_app* app, int32_t cmd) {
    Engine* e = (Engine*)app->userData;
    if (!e) return;

    switch (cmd) {
        case APP_CMD_INIT_WINDOW:
            if (app->window != nullptr) {
                e->createSurface();
                e->hasSurface = true;
            }
            break;
        case APP_CMD_TERM_WINDOW:
            e->destroySurface();
            e->hasSurface = false;
            break;
        case APP_CMD_GAINED_FOCUS:
            e->hasFocus = true;
            break;
        case APP_CMD_LOST_FOCUS:
            e->hasFocus = false;
            break;
        case APP_CMD_WINDOW_RESIZED:
            if (app->window) {
                e->width = ANativeWindow_getWidth(app->window);
                e->height = ANativeWindow_getHeight(app->window);
                if (e->renderer && e->width > 0 && e->height > 0) {
                    e->renderer->resize(e->width, e->height);
                    e->input.setScreenSize(e->width, e->height);
                    e->ui->resize(e->width, e->height);
                }
            }
            break;
        case APP_CMD_LOW_MEMORY:
            LOGW("Low memory warning");
            break;
    }
}

void Engine::createSurface() {
    if (app->window == nullptr) return;

    if (display == EGL_NO_DISPLAY) {
        display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
        eglInitialize(display, nullptr, nullptr);
    }

    const EGLint attribs[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_BLUE_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_RED_SIZE, 8,
        EGL_DEPTH_SIZE, 24,
        EGL_ALPHA_SIZE, 8,
        EGL_NONE
    };

    EGLint numConfigs;
    eglChooseConfig(display, attribs, &config, 1, &numConfigs);

    EGLint format;
    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);
    ANativeWindow_setBuffersGeometry(app->window, 0, 0, format);

    surface = eglCreateWindowSurface(display, config, app->window, nullptr);

    const EGLint contextAttribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 3,
        EGL_NONE
    };
    context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttribs);

    if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
        LOGE("eglMakeCurrent failed");
        return;
    }

    eglQuerySurface(display, surface, EGL_WIDTH, &width);
    eglQuerySurface(display, surface, EGL_HEIGHT, &height);

    LOGI("Surface created: %dx%d", width, height);

    if (!initialized) {
        renderer = std::make_unique<Renderer>();
        if (!renderer->init()) {
            LOGE("Renderer init failed");
            return;
        }

        ui = std::make_unique<UI>();
        ui->init();

        int seed = (int)(std::chrono::system_clock::now().time_since_epoch().count() & 0x7FFFFFFF);
        world = std::make_unique<World>(seed);
        player = std::make_unique<Player>();
        player->position = glm::vec3(8.5f, 120.0f, 8.5f);
        timeOfDay = 0.25f;

        // Get save path from native activity
        if (app->activity && app->activity->internalDataPath) {
            savePath = std::string(app->activity->internalDataPath) + "/world.dat";
        } else {
            savePath = "/data/local/tmp/minecraft_es_world.dat";
        }
        LOGI("Save path: %s", savePath.c_str());

        if (SaveManager::saveExists(savePath)) {
            SaveManager::loadWorld(world.get(), player.get(), savePath);
        }

        // If player fell below world, teleport up
        if (player->position.y < 1.0f) {
            player->position.y = 100.0f;
        }

        initialized = true;
        LOGI("Engine initialized");
    }

    renderer->resize(width, height);
    ui->resize(width, height);
    input.setScreenSize(width, height);

    running = true;
}

void Engine::destroySurface() {
    running = false;

    // Save world
    if (world && player) {
        save();
    }

    if (display != EGL_NO_DISPLAY) {
        eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (context != EGL_NO_CONTEXT) {
            eglDestroyContext(display, context);
            context = EGL_NO_CONTEXT;
        }
        if (surface != EGL_NO_SURFACE) {
            eglDestroySurface(display, surface);
            surface = EGL_NO_SURFACE;
        }
        eglTerminate(display);
        display = EGL_NO_DISPLAY;
    }
}

void Engine::save() {
    if (!world || !player) return;
    SaveManager::saveWorld(world.get(), player.get(), savePath);
}

void Engine::handleInput() {
    // Apply look from touch
    if (input.lookDelta.x != 0.0f || input.lookDelta.y != 0.0f) {
        float sensitivity = 0.25f;
        player->yaw += input.lookDelta.x * sensitivity;
        player->pitch -= input.lookDelta.y * sensitivity;
        player->pitch = glm::clamp(player->pitch, -89.0f, 89.0f);
        input.lookDelta = glm::vec2(0.0f);
    }
}

void Engine::update(float dt) {
    if (!world || !player) return;

    // Day/night cycle - advance slowly
    timeOfDay += dt / dayLength;
    if (timeOfDay > 1.0f) timeOfDay -= 1.0f;

    // Movement input from joystick
    glm::vec3 moveInput(input.joystickDelta.x, 0.0f, -input.joystickDelta.y);
    // Clamp length
    float len = glm::length(glm::vec2(moveInput.x, moveInput.z));
    if (len > 1.0f) {
        moveInput.x /= len;
        moveInput.z /= len;
    }

    bool jump = input.jumpPressed;

    player->update(dt, world.get(), moveInput, jump);

    // Update world (chunk loading)
    world->update(player->position, 6);

    // Block interaction via raycast
    glm::vec3 eye = player->getEyePosition();
    glm::vec3 dir = player->getForward();
    World::RayHit hit = world->raycast(eye, dir, 6.0f);

    if (input.breakPressed) {
        if (hit.hit) {
            BlockType t = world->getBlockAt(hit.x, hit.y, hit.z);
            if (t != BlockType::Bedrock) {
                world->setBlockAt(hit.x, hit.y, hit.z, BlockType::Air);
                LOGI("Broke block at %d,%d,%d", hit.x, hit.y, hit.z);
            }
        }
        input.breakPressed = false;
    }

    if (input.placePressed) {
        if (hit.hit) {
            int px = hit.prevX, py = hit.prevY, pz = hit.prevZ;
            // Check not inside player
            glm::vec3 blockPos((float)px + 0.5f, (float)py + 0.5f, (float)pz + 0.5f);
            glm::vec3 diff = blockPos - player->position;
            bool overlaps = std::abs(diff.x) < 0.8f && std::abs(diff.z) < 0.8f &&
                             (py >= (int)std::floor(player->position.y) &&
                              py <= (int)std::floor(player->position.y + player->height));
            if (!overlaps) {
                world->setBlockAt(px, py, pz, BlockType::Grass);
                LOGI("Placed block at %d,%d,%d", px, py, pz);
            }
        }
        input.placePressed = false;
    }

    // Auto-save periodically
    saveTimer += dt;
    if (saveTimer > 30.0f) {
        saveTimer = 0.0f;
        save();
    }
}

void Engine::render() {
    if (!renderer || !world || !player) return;

    glm::vec3 eye = player->getEyePosition();
    glm::vec3 dir = player->getForward();
    World::RayHit hit = world->raycast(eye, dir, 6.0f);

    LOGI("DBG pos=(%.1f,%.1f,%.1f) hit=%d t=%.2f glErr=0x%x", player->position.x, player->position.y, player->position.z, (int)hit.hit, timeOfDay, glGetError());
    renderer->render(world.get(), player.get(), timeOfDay, hit, hit.hit);
    ui->render(input, player->position, timeOfDay);

    eglSwapBuffers(display, surface);
}

void Engine::frame() {
    auto now = std::chrono::steady_clock::now();
    float dt = std::chrono::duration<float>(now - lastTime).count();
    lastTime = now;

    if (dt > 0.1f) dt = 0.1f;

    input.tick(dt); // detect long-press-to-mine even while the finger stays still
    handleInput();
    update(dt);
    render();
}

void Engine::init() {
    lastTime = std::chrono::steady_clock::now();
}

void Engine::shutdown() {
    save();

    if (renderer) renderer.reset();
    if (world) world.reset();
    if (player) player.reset();
    if (ui) ui.reset();

    destroySurface();
}

extern "C" void android_main(struct android_app* app) {
    app->userData = &gEngine;
    app->onAppCmd = handleCmd;
    app->onInputEvent = handleInput;

    gEngine.app = app;
    gEngine.init();

    while (1) {
        int events;
        struct android_poll_source* source;

        int timeout = gEngine.running ? 0 : -1;

        while (ALooper_pollAll(timeout, nullptr, &events, (void**)&source) >= 0) {
            if (source != nullptr) {
                source->process(app, source);
            }
            if (app->destroyRequested != 0) {
                gEngine.shutdown();
                return;
            }
            timeout = 0;
        }

        if (gEngine.running && gEngine.hasSurface) {
            gEngine.frame();
        }
    }
}
