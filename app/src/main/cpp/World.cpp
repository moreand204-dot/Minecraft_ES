#include "World.h"
#include "Logger.h"
#include <cmath>
#include <fstream>
#include <algorithm>

World::World(int seed) : seed(seed), terrainNoise(seed), biomeNoise(seed * 1337 + 42) {
    // Pre-generate spawn chunks
}

World::~World() {
    for (auto& kv : chunks) {
        kv.second->destroyGL();
    }
    chunks.clear();
}

Chunk* World::getChunk(int cx, int cz) {
    ChunkCoord coord{cx, cz};
    auto it = chunks.find(coord);
    if (it != chunks.end()) return it->second.get();
    return nullptr;
}

Chunk* World::getOrCreateChunk(int cx, int cz) {
    ChunkCoord coord{cx, cz};
    auto it = chunks.find(coord);
    if (it != chunks.end()) return it->second.get();

    auto chunk = std::make_unique<Chunk>(cx, cz);
    Chunk* ptr = chunk.get();
    chunks[coord] = std::move(chunk);

    generateChunk(ptr);
    return ptr;
}

void World::generateChunk(Chunk* chunk) {
    chunk->generateTerrain(this);
    // Mark neighbors dirty for mesh rebuild
    Chunk* left = getChunk(chunk->chunkX - 1, chunk->chunkZ);
    Chunk* right = getChunk(chunk->chunkX + 1, chunk->chunkZ);
    Chunk* back = getChunk(chunk->chunkX, chunk->chunkZ - 1);
    Chunk* front = getChunk(chunk->chunkX, chunk->chunkZ + 1);
    if (left) left->dirty = true;
    if (right) right->dirty = true;
    if (back) back->dirty = true;
    if (front) front->dirty = true;
}

double World::getTerrainHeight(double worldX, double worldZ) const {
    double h = terrainNoise.fbm2D(worldX * 0.01, worldZ * 0.01, 4, 0.5, 2.0);
    // Map to roughly 62..94 range, well above sea level (62) so land dominates
    double height = 78.0 + h * 16.0;

    // Add some hilliness
    double h2 = terrainNoise.fbm2D(worldX * 0.02 + 100, worldZ * 0.02 + 100, 2, 0.5, 2.0);
    height += h2 * 6.0;

    return height;
}

double World::getBiomeNoise(double worldX, double worldZ) const {
    return biomeNoise.fbm2D(worldX * 0.005, worldZ * 0.005, 2, 0.5, 2.0);
}

BlockType World::getBlockAt(int worldX, int worldY, int worldZ) {
    if (worldY < 0 || worldY >= CHUNK_SIZE_Y) return BlockType::Air;
    int cx = worldX >> 4;
    int cz = worldZ >> 4;
    int lx = worldX - (cx << 4);
    int lz = worldZ - (cz << 4);

    Chunk* chunk = getChunk(cx, cz);
    if (!chunk) return BlockType::Air;
    return chunk->getBlock(lx, worldY, lz);
}

void World::setBlockAt(int worldX, int worldY, int worldZ, BlockType type) {
    if (worldY < 0 || worldY >= CHUNK_SIZE_Y) return;
    int cx = worldX >> 4;
    int cz = worldZ >> 4;
    int lx = worldX - (cx << 4);
    int lz = worldZ - (cz << 4);

    Chunk* chunk = getChunk(cx, cz);
    if (!chunk) return;
    chunk->setBlock(lx, worldY, lz, type);

    // Mark neighbors dirty if on edge
    if (lx == 0) { Chunk* c = getChunk(cx - 1, cz); if (c) c->dirty = true; }
    if (lx == CHUNK_SIZE_X - 1) { Chunk* c = getChunk(cx + 1, cz); if (c) c->dirty = true; }
    if (lz == 0) { Chunk* c = getChunk(cx, cz - 1); if (c) c->dirty = true; }
    if (lz == CHUNK_SIZE_Z - 1) { Chunk* c = getChunk(cx, cz + 1); if (c) c->dirty = true; }
}

void World::update(const glm::vec3& playerPos, int viewDistance) {
    int pcx = (int)std::floor(playerPos.x / CHUNK_SIZE_X);
    int pcz = (int)std::floor(playerPos.z / CHUNK_SIZE_Z);

    if (chunksInitialized && pcx == lastPlayerChunkX && pcz == lastPlayerChunkZ) {
        for (auto& kv : chunks) {
            Chunk* c = kv.second.get();
            if (c->generated && c->dirty) {
                c->buildMesh(this);
            }
        }
        return;
    }

    lastPlayerChunkX = pcx;
    lastPlayerChunkZ = pcz;
    chunksInitialized = true;

    // Load chunks in radius
    for (int dx = -viewDistance; dx <= viewDistance; ++dx) {
        for (int dz = -viewDistance; dz <= viewDistance; ++dz) {
            int cx = pcx + dx;
            int cz = pcz + dz;
            ChunkCoord coord{cx, cz};
            if (chunks.find(coord) == chunks.end()) {
                getOrCreateChunk(cx, cz);
            }
        }
    }

    // Unload far chunks
    std::vector<ChunkCoord> toRemove;
    for (auto& kv : chunks) {
        int dx = kv.first.x - pcx;
        int dz = kv.first.z - pcz;
        if (std::abs(dx) > viewDistance + 2 || std::abs(dz) > viewDistance + 2) {
            toRemove.push_back(kv.first);
        }
    }
    for (auto& coord : toRemove) {
        auto it = chunks.find(coord);
        if (it != chunks.end()) {
            it->second->destroyGL();
            chunks.erase(it);
        }
    }

    // Build meshes
    for (auto& kv : chunks) {
        Chunk* c = kv.second.get();
        if (c->generated && c->dirty && c->meshBuilt == false) {
            c->buildMesh(this);
        }
    }
}

void World::render() {
    for (auto& kv : chunks) {
        Chunk* c = kv.second.get();
        if (c->indexCount > 0) {
            glBindVertexArray(c->vao);
            glDrawElements(GL_TRIANGLES, c->indexCount, GL_UNSIGNED_INT, 0);
        }
    }
    glBindVertexArray(0);
}

World::RayHit World::raycast(const glm::vec3& origin, const glm::vec3& direction, float maxDist) {
    RayHit result;
    result.hit = false;
    result.distance = maxDist;

    glm::vec3 dir = glm::normalize(direction);

    int x = (int)std::floor(origin.x);
    int y = (int)std::floor(origin.y);
    int z = (int)std::floor(origin.z);

    int prevX = x, prevY = y, prevZ = z;

    int stepX = (dir.x > 0) ? 1 : -1;
    int stepY = (dir.y > 0) ? 1 : -1;
    int stepZ = (dir.z > 0) ? 1 : -1;

    float tDeltaX = (dir.x != 0) ? std::abs(1.0f / dir.x) : 1e30f;
    float tDeltaY = (dir.y != 0) ? std::abs(1.0f / dir.y) : 1e30f;
    float tDeltaZ = (dir.z != 0) ? std::abs(1.0f / dir.z) : 1e30f;

    float tMaxX = (dir.x > 0) ? ((x + 1 - origin.x) / dir.x) : ((x - origin.x) / dir.x);
    float tMaxY = (dir.y > 0) ? ((y + 1 - origin.y) / dir.y) : ((y - origin.y) / dir.y);
    float tMaxZ = (dir.z > 0) ? ((z + 1 - origin.z) / dir.z) : ((z - origin.z) / dir.z);

    if (dir.x == 0) tMaxX = 1e30f;
    if (dir.y == 0) tMaxY = 1e30f;
    if (dir.z == 0) tMaxZ = 1e30f;

    float t = 0.0f;
    while (t <= maxDist) {
        BlockType type = getBlockAt(x, y, z);
        if (type != BlockType::Air && type != BlockType::Water) {
            result.hit = true;
            result.x = x;
            result.y = y;
            result.z = z;
            result.prevX = prevX;
            result.prevY = prevY;
            result.prevZ = prevZ;
            result.distance = t;
            return result;
        }

        prevX = x; prevY = y; prevZ = z;

        if (tMaxX < tMaxY && tMaxX < tMaxZ) {
            x += stepX;
            t = tMaxX;
            tMaxX += tDeltaX;
        } else if (tMaxY < tMaxZ) {
            y += stepY;
            t = tMaxY;
            tMaxY += tDeltaY;
        } else {
            z += stepZ;
            t = tMaxZ;
            tMaxZ += tDeltaZ;
        }
    }

    return result;
}

void World::saveToFile(const std::string& path) {
    std::ofstream out(path, std::ios::binary);
    if (!out) {
        LOGE("Cannot open save file: %s", path.c_str());
        return;
    }

    // Write header
    const char magic[4] = {'M','E','S','1'};
    out.write(magic, 4);
    out.write(reinterpret_cast<const char*>(&seed), sizeof(seed));

    uint32_t chunkCount = (uint32_t)chunks.size();
    out.write(reinterpret_cast<const char*>(&chunkCount), sizeof(chunkCount));

    for (auto& kv : chunks) {
        Chunk* c = kv.second.get();
        out.write(reinterpret_cast<const char*>(&c->chunkX), sizeof(c->chunkX));
        out.write(reinterpret_cast<const char*>(&c->chunkZ), sizeof(c->chunkZ));
        out.write(reinterpret_cast<const char*>(c->blocks.data()), c->blocks.size());
    }

    LOGI("World saved to %s (%u chunks)", path.c_str(), chunkCount);
}

void World::loadFromFile(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        LOGI("No save file found at %s", path.c_str());
        return;
    }

    char magic[4];
    in.read(magic, 4);
    if (magic[0] != 'M' || magic[1] != 'E' || magic[2] != 'S' || magic[3] != '1') {
        LOGE("Invalid save file");
        return;
    }

    int savedSeed;
    in.read(reinterpret_cast<char*>(&savedSeed), sizeof(savedSeed));

    uint32_t chunkCount;
    in.read(reinterpret_cast<char*>(&chunkCount), sizeof(chunkCount));

    for (uint32_t i = 0; i < chunkCount; ++i) {
        int cx, cz;
        in.read(reinterpret_cast<char*>(&cx), sizeof(cx));
        in.read(reinterpret_cast<char*>(&cz), sizeof(cz));

        ChunkCoord coord{cx, cz};
        auto chunk = std::make_unique<Chunk>(cx, cz);
        in.read(reinterpret_cast<char*>(chunk->blocks.data()), chunk->blocks.size());
        chunk->generated = true;
        chunk->dirty = true;
        chunks[coord] = std::move(chunk);
    }

    LOGI("World loaded from %s (%u chunks)", path.c_str(), chunkCount);
}
