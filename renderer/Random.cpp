// Copyright (C) 2012 Sami Kyöstilä

#include "Random.h"
#include <algorithm>

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
    glm::vec4 result __attribute__((aligned(16)));
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

RandomValue<glm::vec3> Random::generateSpherical()
{
    glm::vec4 sample = generate();
    sample.y = (sample.y + 1) * M_PI;
    float r = sqrtf(1 - sample.x * sample.x);

    glm::vec3 result;
    result.x = r * cosf(sample.y);
    result.y = r * sinf(sample.y);
    result.z = sample.x;
    return RandomValue<glm::vec3>(result, 1 / (4 * M_PI));
}

RandomValue<glm::vec3> Random::generateHemispherical(const glm::vec3& normal)
{
    RandomValue<glm::vec3> result = generateSpherical();
    if (glm::dot(result.value, normal) < 0)
        result.value = -result.value;
    result.probability = 1 / (2 * M_PI);
    return result;
}

RandomValue<glm::vec3> Random::generateCosineHemispherical(const glm::vec3& normal, const glm::vec3& tangent, const glm::vec3& binormal)
{
    glm::vec4 sample = generate();
    float u = .5f * sample.x + .5f;
    float r = sqrtf(u);
    float theta = (sample.y + 1) * M_PI;
    float x = r * cosf(theta);
    float y = r * sinf(theta);
    float z = sqrtf(std::max(0.f, 1.f - u));
    return RandomValue<glm::vec3>(x * tangent + y * binormal + z * normal, cosf(theta) * M_1_PI);
}

RandomValue<glm::vec3> Random::generatePhong(const glm::vec3& normal, float exponent)
{
    // Compact Metallic Reflectance Models
    glm::vec4 sample = generate();
    float u = .5f * sample.x + .5f;
    float v = .5f * sample.y + .5f;
    float a = acosf(powf(u, 1.f / (exponent + 1)));
    float phi = 2 * M_PI * v;

    glm::vec3 result = glm::vec3(sinf(a) * cosf(phi),
                                 sinf(a) * sinf(phi),
                                 cosf(a));
    return RandomValue<glm::vec3>(result, (exponent + 1) / (2 * M_PI) * powf(cosf(a), exponent));
}

RandomValue<bool> Random::russianRoulette(const glm::vec4& probability)
{
    // Note: w component ignored
    float p = std::max(probability.x, std::max(probability.y, probability.z));
    return flipCoin(p);
}

RandomValue<bool> Random::flipCoin(float probability)
{
    float r = generate().x * .5f + .5f;
    if (probability && r <= probability)
        return RandomValue<bool>(true, probability);
    return RandomValue<bool>(false, 1 - probability);
}
