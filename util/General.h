#ifndef UTIL_GENERAL_H
#define UTIL_GENERAL_H

float random_float(const float min = 0.0, const float max = 1.0)
{
    static thread_local std::random_device device;
    static thread_local std::mt19937 generator(device());
    std::uniform_real_distribution<float> distribution(min, max);
    return distribution(generator);
}

#endif
