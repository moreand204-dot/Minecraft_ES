#ifndef PERLIN_NOISE_H
#define PERLIN_NOISE_H

#include <cstdint>
#include <vector>

class PerlinNoise {
public:
    explicit PerlinNoise(int seed);
    double noise2D(double x, double y) const;
    double fbm2D(double x, double y, int octaves, double persistence, double lacunarity) const;
    double noise3D(double x, double y, double z) const;

private:
    std::vector<int> p;
    static double fade(double t);
    static double lerp(double a, double b, double t);
    static double grad(int hash, double x, double y, double z);
    static double grad2D(int hash, double x, double y);
};

#endif
