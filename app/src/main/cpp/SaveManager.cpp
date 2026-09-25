#include "SaveManager.h"
#include "Logger.h"
#include <fstream>
#include <android/log.h>

std::string SaveManager::getSavePath() {
    // NativeActivity provides internal data path via JNI, but for simplicity
    // we'll use /data/local/tmp or app-specific path
    // The main.cpp will set this from native activity internalDataPath
    return "/data/local/tmp/minecraft_es_world.dat";
}

void SaveManager::saveWorld(World* world, Player* player, const std::string& path) {
    world->saveToFile(path);

    // Also save player state
    std::ofstream out(path + ".player", std::ios::binary);
    if (out) {
        out.write(reinterpret_cast<const char*>(&player->position), sizeof(player->position));
        out.write(reinterpret_cast<const char*>(&player->yaw), sizeof(player->yaw));
        out.write(reinterpret_cast<const char*>(&player->pitch), sizeof(player->pitch));
    }
    LOGI("World + player saved");
}

bool SaveManager::loadWorld(World* world, Player* player, const std::string& path) {
    std::ifstream check(path, std::ios::binary);
    if (!check) return false;
    check.close();

    world->loadFromFile(path);

    std::ifstream in(path + ".player", std::ios::binary);
    if (in) {
        in.read(reinterpret_cast<char*>(&player->position), sizeof(player->position));
        in.read(reinterpret_cast<char*>(&player->yaw), sizeof(player->yaw));
        in.read(reinterpret_cast<char*>(&player->pitch), sizeof(player->pitch));
    }
    LOGI("World + player loaded");
    return true;
}

bool SaveManager::saveExists(const std::string& path) {
    std::ifstream check(path, std::ios::binary);
    return check.good();
}
