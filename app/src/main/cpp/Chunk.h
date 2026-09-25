#ifndef CHUNK_H
#define CHUNK_H

#include <GLES3/gl3.h>
#include <glm/glm.hpp>
#include <vector>
#include <cstdint>
#include <cstddef>
#include "Block.h"

constexpr int CHUNK_SIZE_X = 16;
constexpr int CHUNK_SIZE_Y = 256;
constexpr int CHUNK_SIZE_Z = 16;

struct Vertex {
    float x, y, z;
    float u, v;
    float nx, ny, nz;
    float ao;
};

class World;

class Chunk {
public:
    int chunkX, chunkZ;
    bool dirty = true;
    bool generated = false;
    bool meshBuilt = false;

    GLuint vao = 0, vbo = 0, ebo = 0;
    GLsizei indexCount = 0;

    std::vector<uint8_t> blocks;

    Chunk(int cx, int cz);

    BlockType getBlock(int x, int y, int z) const;
    void setBlock(int x, int y, int z, BlockType type);
    bool isInside(int x, int y, int z) const;

    void generateTerrain(World* world);
    void buildMesh(World* world);
    void uploadMesh();
    void destroyGL();

private:
    static int index(int x, int y, int z) {
        return (y * CHUNK_SIZE_Z + z) * CHUNK_SIZE_X + x;
    }

    void addFace(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices,
                 float bx, float by, float bz, int face, BlockType type, World* world);
};

#endif
