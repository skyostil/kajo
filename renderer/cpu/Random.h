// Copyright (C) 2012 Sami Ky�stil�
#ifndef CPU_RANDOM_H
#define CPU_RANDOM_H

#include <glm/glm.hpp>
#include "renderer/Util.h"

#if defined(USE_SSE2)
#    include <pmmintrin.h>
#else
#    include <stdint.h>
#endif

namespace cpu
{

template <typename T>
struct RandomValue
{
    RandomValue():
        probability(0)
    {
    }

    explicit RandomValue(T value):
        value(value), probability(1)
    {
    }

    RandomValue(T value, float probability):
        value(value), probability(probability)
    {
    }

    T value;
    float probability;
};

// TODO: Use alignas(__m128i) once the compilers support it
class Random: public NonCopyable
{
public:
    Random(unsigned seed = 0715517);

    void setSeed(unsigned seed);

    /**
     *  Generates a vector of random numbers with approximately uniform
     *  distribution [-1..1]
     */
    glm::vec4 generate();

    RandomValue<glm::vec3> generateSpherical();
    RandomValue<glm::vec3> generateHemispherical(const glm::vec3& normal);
    RandomValue<glm::vec3> generateCosineHemispherical();
    RandomValue<glm::vec3> generatePhong(float exponent);

    RandomValue<bool> flipCoin(float probability);
    RandomValue<bool> russianRoulette(const glm::vec4& probability);

private:
#if defined(USE_SSE2)
    __m128i m_state;
#else
    uint32_t m_high;
    uint32_t m_low;
#endif
} __attribute__((aligned(16)));

}

#endif // RANDOM_H
