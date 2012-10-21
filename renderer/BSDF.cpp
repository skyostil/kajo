// Copyright (C) 2012 Sami Kyöstilä

#include "BSDF.h"
#include "Random.h"
#include "SurfacePoint.h"

BSDF::BSDF(const SurfacePoint* surfacePoint):
    m_surfacePoint(surfacePoint)
{
}

LambertBSDF::LambertBSDF(const SurfacePoint* surfacePoint, const glm::vec4& color):
    BSDF(surfacePoint),
    m_color(color)
{
}

RandomValue<glm::vec3> LambertBSDF::generateSample(Random& random) const
{
    //return random.generateSpherical();
    return random.generateHemispherical(m_surfacePoint->normal);
    //return random.generateCosineHemispherical(m_surfacePoint->normal, m_surfacePoint->tangent, m_surfacePoint->binormal);
}

glm::vec4 LambertBSDF::evaluateSample(const glm::vec3& direction) const
{
    return m_color * M_1_PI;
}

float LambertBSDF::sampleProbability(const glm::vec3& direction) const
{
    // Uniform BSDF over the hemisphere.
    float hemisphereArea = 4 * M_PI / 2;
    return 1 / hemisphereArea;
}

PhongBSDF::PhongBSDF(const SurfacePoint* surfacePoint, const glm::vec4& color, float exponent):
    BSDF(surfacePoint),
    m_color(color),
    m_exponent(exponent)
{
}

RandomValue<glm::vec3> PhongBSDF::generateSample(Random& random) const
{
    RandomValue<glm::vec3> result = random.generatePhong(m_surfacePoint->normal, m_exponent);

    // Rotate the vector to point along the reflection.
    glm::vec3 reflection = glm::reflect(m_surfacePoint->view, m_surfacePoint->normal);
    //glm::vec3 vr = glm::vec3(generate());
    glm::vec3 vr(0, 0, 1);
    glm::vec3 vu = glm::normalize(glm::cross(vr, reflection));
    glm::vec3 vv = glm::cross(vu, reflection);
    glm::mat3 rot = glm::mat3(vu, vv, reflection);
    result.value = rot * result.value;
    return result;
}

glm::vec4 PhongBSDF::evaluateSample(const glm::vec3& direction) const
{
    glm::vec3 reflection = glm::reflect(m_surfacePoint->view, m_surfacePoint->normal);
    float cos_a = std::max(0.f, glm::dot(reflection, direction));
    float sin_a = sqrtf(std::max(0.f, 1 - cos_a * cos_a));
    return (m_exponent + 1) / (2 * M_PI) * m_color * powf(cos_a, m_exponent) * sin_a;
}

float PhongBSDF::sampleProbability(const glm::vec3& direction) const
{
    glm::vec3 reflection = glm::reflect(m_surfacePoint->view, m_surfacePoint->normal);
    float cos_a = std::max(0.f, glm::dot(reflection, direction));
    float sin_a = sqrtf(std::max(0.f, 1 - cos_a * cos_a));
    return (m_exponent + 1) / (2 * M_PI) * powf(cos_a, m_exponent) * sin_a;
}

IdealReflectorBSDF::IdealReflectorBSDF(const SurfacePoint* surfacePoint, const glm::vec4& color):
    BSDF(surfacePoint),
    m_color(color)
{
}

RandomValue<glm::vec3> IdealReflectorBSDF::generateSample(Random& random) const
{
    return RandomValue<glm::vec3>(glm::reflect(m_surfacePoint->view, m_surfacePoint->normal), 1);
}

glm::vec4 IdealReflectorBSDF::evaluateSample(const glm::vec3& direction) const
{
    return m_color;
}

float IdealReflectorBSDF::sampleProbability(const glm::vec3& direction) const
{
    return 0.f;
}
