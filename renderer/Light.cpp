// Copyright (C) 2012 Sami Kyöstilä

#include "Light.h"
#include "Random.h"
#include "SurfacePoint.h"
#include "scene/Scene.h"

Light::Light(const SurfacePoint* surfacePoint):
    m_surfacePoint(surfacePoint)
{
}

SphericalLight::SphericalLight(const SurfacePoint* surfacePoint, const scene::Sphere* sphere,
                               const TransformData* transformData, const glm::vec4& emission):
    Light(surfacePoint),
    m_sphere(sphere),
    m_transformData(transformData),
    m_emission(emission)
{
}

RandomValue<glm::vec3> SphericalLight::generateSample(Random& random) const
{
    glm::vec3 lightPos(m_sphere->transform * glm::vec4(0, 0, 0, 1));
    glm::vec3 w = lightPos - m_surfacePoint->position;
    glm::vec3 u = glm::normalize(glm::cross(fabs(w.x) > .1 ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0), w));
    glm::vec3 v = glm::cross(w, u);
    float cos_a_max = sqrtf(1 - m_sphere->radius * m_sphere->radius / glm::dot(w, w));
    glm::vec4 randomSample = random.generate();
    float s1 = (randomSample.x * .5f) + .5f;
    float s2 = (randomSample.y * .5f) + .5f;
    float cos_a = 1 - s1 + s1 * cos_a_max;
    float sin_a = sqrtf(1 - cos_a * cos_a);
    float phi = 2 * M_PI * s2;

    RandomValue<glm::vec3> result;
    result.value = glm::normalize(u * cosf(phi) * sin_a + v * sinf(phi) * sin_a + w * cos_a);
    result.probability = 1 / ((1 - cos_a_max) * (2 * M_PI));
    return result;
}

glm::vec4 SphericalLight::evaluateSample(const glm::vec3& direction) const
{
    return m_emission;
}

float SphericalLight::sampleProbability(const glm::vec3& direction) const
{
    glm::vec3 lightPos(m_sphere->transform * glm::vec4(0, 0, 0, 1));
    glm::vec3 w = lightPos - m_surfacePoint->position;
    float cos_a_max = sqrtf(1 - m_sphere->radius * m_sphere->radius / glm::dot(w, w));
    float cos_a = glm::dot(direction, glm::normalize(w));

    if (cos_a < cos_a_max)
        return 0;
    return 1.f / ((1 - cos_a_max) * (2 * M_PI));
}
