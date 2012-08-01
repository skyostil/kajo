// Copyright (C) 2012 Sami Kyöstilä
#ifndef RANDOM_H
#define RANDOM_H

#include <glm/glm.hpp>

#if defined(USE_SSE2)
#    include <pmmintrin.h>
#else
#    include <stdint.h>
#endif

// TODO: Use alignas(__m128i) once the compilers support it
class Random
{
public:
    Random(unsigned seed = 0715517);

    void setSeed(unsigned seed);

    /**
     *  Generates a vector of random numbers with approximately uniform
     *  distribution [-1..1]
     */
    glm::vec4 generate();

    glm::vec3 generateSpherical();
private:
#if defined(USE_SSE2)
    __m128i m_state;
#else
    uint32_t m_high;
    uint32_t m_low;
#endif
} __attribute__((aligned(16)));

#endif // RANDOM_H
