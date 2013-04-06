// Copyright (C) 2012 Sami Kyöstilä

#include "BSDF.h"
#include "Random.h"
#include "SurfacePoint.h"

using namespace cpu;

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
    RandomValue<glm::vec3> result = random.generateCosineHemispherical();
    result.value =
        m_surfacePoint->tangent * result.value.x +
        m_surfacePoint->binormal * result.value.y +
        m_surfacePoint->normal * result.value.z;
    return result;
}

glm::vec4 LambertBSDF::evaluateSample(const glm::vec3& direction) const
{
    return m_color * M_1_PI;
}

float LambertBSDF::sampleProbability(const glm::vec3& direction) const
{
    float cos_theta = glm::dot(direction, m_surfacePoint->normal);
    return M_1_PI * cos_theta;
}

PhongBSDF::PhongBSDF(const SurfacePoint* surfacePoint, const glm::vec4& color, float exponent):
    BSDF(surfacePoint),
    m_color(color),
    m_exponent(exponent)
{
}

RandomValue<glm::vec3> PhongBSDF::generateSample(Random& random) const
{
    RandomValue<glm::vec3> result = random.generatePhong(m_exponent);

    // Rotate the vector to point along the reflection.
    glm::vec3 reflection = glm::reflect(m_surfacePoint->view, m_surfacePoint->normal);
    //glm::vec3 vr = glm::vec3(generate());
    glm::vec3 r(0, 0, 1);
    glm::vec3 u = glm::normalize(glm::cross(r, reflection));
    glm::vec3 v = glm::cross(u, reflection);
    result.value = glm::mat3(u, v, reflection) * result.value;
    return result;
}

glm::vec4 PhongBSDF::evaluateSample(const glm::vec3& direction) const
{
    glm::vec3 reflection = glm::reflect(m_surfacePoint->view, m_surfacePoint->normal);
    float cos_a = std::max(0.f, glm::dot(reflection, direction));
    return (m_exponent + 1) / (2 * M_PI) * m_color * powf(cos_a, m_exponent);
}

float PhongBSDF::sampleProbability(const glm::vec3& direction) const
{
    glm::vec3 reflection = glm::reflect(m_surfacePoint->view, m_surfacePoint->normal);
    float cos_a = std::max(0.f, glm::dot(reflection, direction));
    return (m_exponent + 1) / (2 * M_PI) * powf(cos_a, m_exponent);
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
    float cos_a = std::max(0.f, glm::dot(direction, m_surfacePoint->normal));
    return m_color / cos_a;
}

float IdealReflectorBSDF::sampleProbability(const glm::vec3& direction) const
{
    return 0.f;
}

IdealTransmissionBSDF::IdealTransmissionBSDF(const SurfacePoint* surfacePoint, const glm::vec4& color,
                                             float refractiveIndex):
    BSDF(surfacePoint),
    m_color(color),
    m_refractiveIndex(refractiveIndex)
{
}

RandomValue<glm::vec3> IdealTransmissionBSDF::generateSample(Random& random) const
{
    float cos_a = glm::dot(m_surfacePoint->view, m_surfacePoint->normal);
    bool enteringMaterial = (cos_a < 0);
    glm::vec3 normal = enteringMaterial ? m_surfacePoint->normal : -m_surfacePoint->normal;
    float airRefractiveIndex = 1;
    float eta = enteringMaterial ? airRefractiveIndex / m_refractiveIndex :
                                   m_refractiveIndex / airRefractiveIndex;
    cos_a = glm::dot(m_surfacePoint->view, normal);

    // Total internal reflection
    RandomValue<glm::vec3> result;
    if (1 - eta * eta * (1 - cos_a * cos_a) < 0) {
        result.value = glm::reflect(m_surfacePoint->view, normal);
    } else {
        result.value = glm::refract(m_surfacePoint->view, normal, eta);
    }
    result.probability = 1.f;
    return result;
}

glm::vec4 IdealTransmissionBSDF::evaluateSample(const glm::vec3& direction) const
{
    float cos_a = std::abs(glm::dot(direction, m_surfacePoint->normal));
    return m_color / cos_a;
}

float IdealTransmissionBSDF::sampleProbability(const glm::vec3& direction) const
{
    return 0.f;
}
