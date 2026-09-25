#ifndef SAVE_MANAGER_H
#define SAVE_MANAGER_H

#include <string>
#include "World.h"
#include "Player.h"

class SaveManager {
public:
    static std::string getSavePath();
    static void saveWorld(World* world, Player* player, const std::string& path);
    static bool loadWorld(World* world, Player* player, const std::string& path);
    static bool saveExists(const std::string& path);
};

#endif
