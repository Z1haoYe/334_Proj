#include <vector>
#include <random>
#include <numeric>
#include <chrono>

#include "perlin.hpp"
// Define a random number generator
std::mt19937 rng;

// Define the permutation array
std::vector<int> permutation;

// Function to initialize the permutation array
void init_permutation() {

    auto seed = std::chrono::system_clock::now().time_since_epoch().count();
    rng.seed(seed);
    // Fill the array with values from 0 to 255
    permutation.resize(256);
    std::iota(permutation.begin(), permutation.end(), 0);
    // Shuffle the array using the random number generator
    std::shuffle(permutation.begin(), permutation.end(), rng);
    // Duplicate the array to avoid index overflow
    permutation.insert(permutation.end(), permutation.begin(), permutation.end());
}

float lerp(float a, float b, float t) {
    return a + t * (b - a);
}

float fade(float t) {
    return t * t * t * (t * (t * 6 - 15) + 10);
}

float grad(int hash, float x, float y, float z) {
    int h = hash & 15;
    float u = h < 8 ? x : y;
    float v = h < 4 ? y : h == 12 || h == 14 ? x : z;
    return ((h & 1) ? -u : u) + ((h & 2) ? -v : v);
}

float perlin_noise_2d(float x, float y) {
    int X = (int)floor(x) & 255;
    int Y = (int)floor(y) & 255;
    x -= floor(x);
    y -= floor(y);
    float u = fade(x);
    float v = fade(y);
    int A = (permutation[X] + Y) & 255;
    int B = (permutation[X + 1] + Y) & 255;
    return lerp(lerp(grad(permutation[A], x, y, 0),
        grad(permutation[B], x - 1, y, 0), u),
        lerp(grad(permutation[A + 1], x, y - 1, 0),
            grad(permutation[B + 1], x - 1, y - 1, 0), u), v);
}