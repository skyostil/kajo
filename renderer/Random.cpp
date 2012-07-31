// Copyright (C) 2012 Sami Kyöstilä
#include "Random.h"

Random::Random(unsigned seed)
{
    setSeed(seed);
}

void Random::setSeed(unsigned seed)
{
#if defined(USE_SSE2)
    m_state = _mm_set1_epi16(seed);
#else
    m_high = seed;
    m_low = seed ^ 0x49616E42;
#endif
}

#if defined(USE_SSE2)
static const __m128 g_invScale = _mm_set1_ps(1.0f / 0x7fffffff);
#endif

glm::vec4 Random::generate()
{
    glm::vec4 result;
#if defined(USE_SSE2)
    __m128i tmp1, tmp2;
    __m128 res;

    // Shuffle and add
    tmp1 = _mm_shufflelo_epi16(m_state, 0x1e);
    tmp2 = _mm_shufflehi_epi16(m_state, 0x1e);
    tmp1 = _mm_unpackhi_epi64(tmp1, tmp2);
    m_state = _mm_add_epi64(m_state, tmp1);

    // Convert result to floating point
    res = _mm_cvtepi32_ps(m_state);
    *reinterpret_cast<__m128*>(&result) = _mm_mul_ps(res, g_invScale);
#else
    for (int i = 0; i < 4; i++)
    {
        m_high = (m_high << 16) + (m_high >> 16);
        m_high += m_low;
        m_low += m_high;
        result[i] = static_cast<int32_t>(m_high) / float(0x7fffffff);
    }
#endif
    return result;
}

glm::vec3 Random::generateSpherical()
{
    glm::vec4 sample(generate());
    glm::vec3 result;

    result.z = sample.x;
    float phi = (result.y * .5f + .5) * 2 * M_PI;
    float theta = asinf(result.z);
    result.x = cosf(theta) * cosf(phi);
    result.y = cosf(theta) * sinf(phi);
    return result;
}
