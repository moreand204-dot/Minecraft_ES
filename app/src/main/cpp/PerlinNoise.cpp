#include "PerlinNoise.h"
#include <numeric>
#include <algorithm>
#include <random>
#include <cmath>

PerlinNoise::PerlinNoise(int seed) {
    p.resize(256);
    std::iota(p.begin(), p.end(), 0);
    std::mt19937 engine(static_cast<uint32_t>(seed));
    std::shuffle(p.begin(), p.end(), engine);
    p.insert(p.end(), p.begin(), p.end());
}

double PerlinNoise::fade(double t) {
    return t * t * t * (t * (t * 6 - 15) + 10);
}

double PerlinNoise::lerp(double a, double b, double t) {
    return a + t * (b - a);
}

double PerlinNoise::grad(int hash, double x, double y, double z) {
    int h = hash & 15;
    double u = h < 8 ? x : y;
    double v = h < 4 ? y : (h == 12 || h == 14 ? x : z);
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

double PerlinNoise::grad2D(int hash, double x, double y) {
    int h = hash & 7;
    double u = h < 4 ? x : y;
    double v = h < 4 ? y : x;
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

double PerlinNoise::noise2D(double x, double y) const {
    int X = (int)std::floor(x) & 255;
    int Y = (int)std::floor(y) & 255;

    x -= std::floor(x);
    y -= std::floor(y);

    double u = fade(x);
    double v = fade(y);

    int A = p[X] + Y;
    int B = p[X + 1] + Y;

    double res = lerp(
        lerp(grad2D(p[A], x, y), grad2D(p[B], x - 1, y), u),
        lerp(grad2D(p[A + 1], x, y - 1), grad2D(p[B + 1], x - 1, y - 1), u),
        v);
    return res;
}

double PerlinNoise::noise3D(double x, double y, double z) const {
    int X = (int)std::floor(x) & 255;
    int Y = (int)std::floor(y) & 255;
    int Z = (int)std::floor(z) & 255;

    x -= std::floor(x);
    y -= std::floor(y);
    z -= std::floor(z);

    double u = fade(x);
    double v = fade(y);
    double w = fade(z);

    int A = p[X] + Y;
    int AA = p[A] + Z;
    int AB = p[A + 1] + Z;
    int B = p[X + 1] + Y;
    int BA = p[B] + Z;
    int BB = p[B + 1] + Z;

    double res = lerp(
        lerp(
            lerp(grad(p[AA], x, y, z), grad(p[BA], x - 1, y, z), u),
            lerp(grad(p[AB], x, y - 1, z), grad(p[BB], x - 1, y - 1, z), u),
            v),
        lerp(
            lerp(grad(p[AA + 1], x, y, z - 1), grad(p[BA + 1], x - 1, y, z - 1), u),
            lerp(grad(p[AB + 1], x, y - 1, z - 1), grad(p[BB + 1], x - 1, y - 1, z - 1), u),
            v),
        w);
    return res;
}

double PerlinNoise::fbm2D(double x, double y, int octaves, double persistence, double lacunarity) const {
    double total = 0.0;
    double frequency = 1.0;
    double amplitude = 1.0;
    double maxValue = 0.0;

    for (int i = 0; i < octaves; ++i) {
        total += noise2D(x * frequency, y * frequency) * amplitude;
        maxValue += amplitude;
        amplitude *= persistence;
        frequency *= lacunarity;
    }
    return total / maxValue;
}
