#ifndef WORLD_H
#define WORLD_H

#include <glm/glm.hpp>
#include <unordered_map>
#include <memory>
#include <vector>
#include <mutex>
#include "Chunk.h"
#include "PerlinNoise.h"

struct ChunkCoord {
    int x, z;
    bool operator==(const ChunkCoord& o) const { return x == o.x && z == o.z; }
};

struct ChunkCoordHash {
    std::size_t operator()(const ChunkCoord& c) const {
        return std::hash<int>()(c.x) ^ (std::hash<int>()(c.z) << 1);
    }
};

class World {
public:
    World(int seed);
    ~World();

    int getSeed() const { return seed; }

    void update(const glm::vec3& playerPos, int viewDistance);
    void render();

    BlockType getBlockAt(int worldX, int worldY, int worldZ);
    void setBlockAt(int worldX, int worldY, int worldZ, BlockType type);

    double getTerrainHeight(double worldX, double worldZ) const;
    double getBiomeNoise(double worldX, double worldZ) const;

    // Raycast for block breaking/placing
    struct RayHit {
        bool hit;
        int x, y, z;
        int prevX, prevY, prevZ;
        float distance;
    };
    RayHit raycast(const glm::vec3& origin, const glm::vec3& direction, float maxDist);

    Chunk* getChunk(int cx, int cz);
    Chunk* getOrCreateChunk(int cx, int cz);

    // Save/load
    void saveToFile(const std::string& path);
    void loadFromFile(const std::string& path);

    std::mutex chunkMutex;

private:
    int seed;
    PerlinNoise terrainNoise;
    PerlinNoise biomeNoise;

    std::unordered_map<ChunkCoord, std::unique_ptr<Chunk>, ChunkCoordHash> chunks;
    std::vector<ChunkCoord> chunksToUnload;

    int lastPlayerChunkX = 0;
    int lastPlayerChunkZ = 0;
    bool chunksInitialized = false;

    void generateChunk(Chunk* chunk);
};

#endif
