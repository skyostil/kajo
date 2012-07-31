// Copyright (C) 2012 Sami Kyöstilä
#ifndef RANDOM_H
#define RANDOM_H

#include <glm/glm.hpp>

#if defined(USE_VECTORIZED_MATH)
#    define USE_SSE2
#endif

#if defined(USE_SSE2)
#    include <pmmintrin.h>
#endif

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
private:
#if defined(USE_SSE2)
    __m128i m_state;
#else
    uint32_t m_high;
    uint32_t m_low;
#endif
};

#endif // RANDOM_H
