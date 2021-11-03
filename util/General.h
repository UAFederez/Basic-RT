#ifndef UTIL_GENERAL_H
#define UTIL_GENERAL_H

#include <chrono>

float random_float(const float min = 0.0, const float max = 1.0)
{
    static std::mt19937 rng(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    std::uniform_real_distribution<float> distribution(min, max);
    return distribution(rng);
}

#endif
