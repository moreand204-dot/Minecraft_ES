#include "Chunk.h"
#include "World.h"
#include "Logger.h"
#include <cmath>
#include <cstring>
#include <cstddef>

Chunk::Chunk(int cx, int cz) : chunkX(cx), chunkZ(cz) {
    blocks.resize(CHUNK_SIZE_X * CHUNK_SIZE_Y * CHUNK_SIZE_Z, 0);
}

bool Chunk::isInside(int x, int y, int z) const {
    return x >= 0 && x < CHUNK_SIZE_X && y >= 0 && y < CHUNK_SIZE_Y && z >= 0 && z < CHUNK_SIZE_Z;
}

BlockType Chunk::getBlock(int x, int y, int z) const {
    if (!isInside(x, y, z)) return BlockType::Air;
    return static_cast<BlockType>(blocks[index(x, y, z)]);
}

void Chunk::setBlock(int x, int y, int z, BlockType type) {
    if (!isInside(x, y, z)) return;
    blocks[index(x, y, z)] = static_cast<uint8_t>(type);
    dirty = true;
}

void Chunk::generateTerrain(World* world) {
    int baseX = chunkX * CHUNK_SIZE_X;
    int baseZ = chunkZ * CHUNK_SIZE_Z;

    for (int x = 0; x < CHUNK_SIZE_X; ++x) {
        for (int z = 0; z < CHUNK_SIZE_Z; ++z) {
            double worldX = (double)(baseX + x);
            double worldZ = (double)(baseZ + z);

            double height = world->getTerrainHeight(worldX, worldZ);
            int terrainHeight = (int)(height);

            double biome = world->getBiomeNoise(worldX, worldZ);

            for (int y = 0; y < CHUNK_SIZE_Y; ++y) {
                BlockType type = BlockType::Air;
                if (y == 0) {
                    type = BlockType::Bedrock;
                } else if (y < terrainHeight - 4) {
                    type = BlockType::Stone;
                } else if (y < terrainHeight) {
                    type = (terrainHeight <= 62) ? BlockType::Sand : BlockType::Dirt;
                } else if (y == terrainHeight) {
                    if (terrainHeight <= 62) {
                        type = BlockType::Sand;
                    } else if (biome > 0.3) {
                        type = BlockType::Sand;
                    } else {
                        type = BlockType::Grass;
                    }
                } else if (y <= 62) {
                    type = BlockType::Water;
                }

                blocks[index(x, y, z)] = static_cast<uint8_t>(type);
            }

            if (terrainHeight > 63 && terrainHeight < 200) {
                uint32_t h = (uint32_t)((baseX + x) * 73856093u) ^ (uint32_t)((baseZ + z) * 19349663u);
                if ((h % 100) < 3) {
                    int treeBase = terrainHeight + 1;
                    for (int ty = 0; ty < 5 && treeBase + ty < CHUNK_SIZE_Y; ++ty) {
                        blocks[index(x, treeBase + ty, z)] = static_cast<uint8_t>(BlockType::Wood);
                    }
                    for (int lx = -2; lx <= 2; ++lx) {
                        for (int lz = -2; lz <= 2; ++lz) {
                            for (int ly = 3; ly <= 5; ++ly) {
                                int px = x + lx;
                                int py = treeBase + ly;
                                int pz = z + lz;
                                if (!isInside(px, py, pz)) continue;
                                if (std::abs(lx) == 2 && std::abs(lz) == 2 && ly == 5) continue;
                                if (blocks[index(px, py, pz)] == 0) {
                                    blocks[index(px, py, pz)] = static_cast<uint8_t>(BlockType::Leaves);
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    generated = true;
    dirty = true;
}

void Chunk::addFace(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices,
                    float bx, float by, float bz, int face, BlockType type, World* world) {
    (void)world;
    int texIndex = Block::getTextureIndex(type, (face == 0) ? 0 : (face == 1 ? 1 : 2));
    int atlasCols = 4;
    int tx = texIndex % atlasCols;
    int ty = texIndex / atlasCols;
    float uvSize = 1.0f / atlasCols;
    float u0 = tx * uvSize;
    float v0 = ty * uvSize;
    float u1 = u0 + uvSize;
    float v1 = v0 + uvSize;

    float light = 1.0f;
    float nx = 0, ny = 0, nz = 0;

    Vertex v[4];

    switch (face) {
        case 0: // top
            ny = 1;
            v[0] = {bx,     by+1, bz,   u0, v0, nx,ny,nz, light};
            v[1] = {bx+1,   by+1, bz,   u1, v0, nx,ny,nz, light};
            v[2] = {bx+1,   by+1, bz+1, u1, v1, nx,ny,nz, light};
            v[3] = {bx,     by+1, bz+1, u0, v1, nx,ny,nz, light};
            break;
        case 1: // bottom
            ny = -1;
            v[0] = {bx,     by, bz,   u0, v0, nx,ny,nz, light};
            v[1] = {bx+1,   by, bz,   u1, v0, nx,ny,nz, light};
            v[2] = {bx+1,   by, bz+1, u1, v1, nx,ny,nz, light};
            v[3] = {bx,     by, bz+1, u0, v1, nx,ny,nz, light};
            break;
        case 2: // +Z
            nz = 1;
            v[0] = {bx,     by,   bz+1, u0, v1, nx,ny,nz, light};
            v[1] = {bx+1,   by,   bz+1, u1, v1, nx,ny,nz, light};
            v[2] = {bx+1,   by+1, bz+1, u1, v0, nx,ny,nz, light};
            v[3] = {bx,     by+1, bz+1, u0, v0, nx,ny,nz, light};
            break;
        case 3: // -Z
            nz = -1;
            v[0] = {bx+1,   by,   bz,   u0, v1, nx,ny,nz, light};
            v[1] = {bx,     by,   bz,   u1, v1, nx,ny,nz, light};
            v[2] = {bx,     by+1, bz,   u1, v0, nx,ny,nz, light};
            v[3] = {bx+1,   by+1, bz,   u0, v0, nx,ny,nz, light};
            break;
        case 4: // +X
            nx = 1;
            v[0] = {bx+1,   by,   bz+1, u0, v1, nx,ny,nz, light};
            v[1] = {bx+1,   by,   bz,   u1, v1, nx,ny,nz, light};
            v[2] = {bx+1,   by+1, bz,   u1, v0, nx,ny,nz, light};
            v[3] = {bx+1,   by+1, bz+1, u0, v0, nx,ny,nz, light};
            break;
        case 5: // -X
            nx = -1;
            v[0] = {bx,     by,   bz,   u0, v1, nx,ny,nz, light};
            v[1] = {bx,     by,   bz+1, u1, v1, nx,ny,nz, light};
            v[2] = {bx,     by+1, bz+1, u1, v0, nx,ny,nz, light};
            v[3] = {bx,     by+1, bz,   u0, v0, nx,ny,nz, light};
            break;
    }

    uint32_t base = (uint32_t)vertices.size();
    for (int i = 0; i < 4; ++i) vertices.push_back(v[i]);
    indices.push_back(base + 0);
    indices.push_back(base + 1);
    indices.push_back(base + 2);
    indices.push_back(base + 0);
    indices.push_back(base + 2);
    indices.push_back(base + 3);
}

void Chunk::buildMesh(World* world) {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    vertices.reserve(4096);
    indices.reserve(6144);

    int worldOffX = chunkX * CHUNK_SIZE_X;
    int worldOffZ = chunkZ * CHUNK_SIZE_Z;

    for (int y = 0; y < CHUNK_SIZE_Y; ++y) {
        for (int z = 0; z < CHUNK_SIZE_Z; ++z) {
            for (int x = 0; x < CHUNK_SIZE_X; ++x) {
                BlockType type = getBlock(x, y, z);
                if (type == BlockType::Air) continue;

                float bx = (float)(worldOffX + x);
                float by = (float)y;
                float bz = (float)(worldOffZ + z);

                int worldX = worldOffX + x;
                int worldZ = worldOffZ + z;

                // Top
                if (y + 1 < CHUNK_SIZE_Y) {
                    BlockType n = getBlock(x, y + 1, z);
                    if (Block::isTransparent(n) && n != type)
                        addFace(vertices, indices, bx, by, bz, 0, type, world);
                } else {
                    addFace(vertices, indices, bx, by, bz, 0, type, world);
                }

                // Bottom
                if (y - 1 >= 0) {
                    BlockType n = getBlock(x, y - 1, z);
                    if (Block::isTransparent(n) && n != type)
                        addFace(vertices, indices, bx, by, bz, 1, type, world);
                }

                // +Z
                BlockType nz1 = (z + 1 < CHUNK_SIZE_Z) ? getBlock(x, y, z + 1)
                                                        : world->getBlockAt(worldX, y, worldZ + 1);
                if (Block::isTransparent(nz1) && nz1 != type)
                    addFace(vertices, indices, bx, by, bz, 2, type, world);

                // -Z
                BlockType nz2 = (z - 1 >= 0) ? getBlock(x, y, z - 1)
                                              : world->getBlockAt(worldX, y, worldZ - 1);
                if (Block::isTransparent(nz2) && nz2 != type)
                    addFace(vertices, indices, bx, by, bz, 3, type, world);

                // +X
                BlockType nx1 = (x + 1 < CHUNK_SIZE_X) ? getBlock(x + 1, y, z)
                                                        : world->getBlockAt(worldX + 1, y, worldZ);
                if (Block::isTransparent(nx1) && nx1 != type)
                    addFace(vertices, indices, bx, by, bz, 4, type, world);

                // -X
                BlockType nx2 = (x - 1 >= 0) ? getBlock(x - 1, y, z)
                                              : world->getBlockAt(worldX - 1, y, worldZ);
                if (Block::isTransparent(nx2) && nx2 != type)
                    addFace(vertices, indices, bx, by, bz, 5, type, world);
            }
        }
    }

    if (vertices.empty()) {
        meshBuilt = true;
        dirty = false;
        if (vao != 0) {
            glDeleteVertexArrays(1, &vao);
            glDeleteBuffers(1, &vbo);
            glDeleteBuffers(1, &ebo);
            vao = 0; vbo = 0; ebo = 0;
            indexCount = 0;
        }
        return;
    }

    if (vao == 0) {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);
    }

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, x));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, u));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, nx));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, ao));

    glBindVertexArray(0);

    indexCount = (GLsizei)indices.size();
    meshBuilt = true;
    dirty = false;
}

void Chunk::uploadMesh() {
    // No-op
}

void Chunk::destroyGL() {
    if (vao != 0) {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &ebo);
        vao = 0; vbo = 0; ebo = 0;
        indexCount = 0;
    }
}
