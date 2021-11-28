#include <random>

uint32 getRandomInt(uint32 low, uint32 high)
{
    std::default_random_engine generator;
    std::uniform_int_distribution<uint32> distribution(low, high);
    return distribution(generator);
}

float getRandomReal(float low, float high)
{
    std::default_random_engine generator;
    std::uniform_real_distribution<float> distribution(low, high);
    return distribution(generator);
}