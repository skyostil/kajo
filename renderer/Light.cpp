// Copyright (C) 2012 Sami Kyöstilä

#include "Light.h"
#include "Random.h"
#include "Ray.h"
#include "Raytracer.h"
#include "SurfacePoint.h"
#include "scene/Scene.h"

Light::Light(const SurfacePoint* surfacePoint, const Raytracer* raytracer):
    m_surfacePoint(surfacePoint),
    m_raytracer(raytracer)
{
}

SphericalLight::SphericalLight(const SurfacePoint* surfacePoint, const Raytracer* raytracer,
                               const scene::Sphere* sphere,
                               const TransformData* transformData, const glm::vec4& emission):
    Light(surfacePoint, raytracer),
    m_sphere(sphere),
    m_transformData(transformData),
    m_emission(emission)
{
}

float SphericalLight::solidAngle(const glm::vec3& lightPos) const
{
    float dist = glm::length(lightPos - m_surfacePoint->position);
    if (dist < m_sphere->radius)
        return 4 * M_PI; // Full sphere
    return 2 * M_PI * (1 - cosf(asinf(m_sphere->radius / dist)));
}

RandomValue<glm::vec3> SphericalLight::generateSample(Random& random) const
{
    // From "Lightcuts: A Scalable Approach to Illumination"
    glm::vec3 lightPos(m_sphere->transform * glm::vec4(0, 0, 0, 1));
    glm::vec4 randomSample = random.generate();
    float s1 = (randomSample.x * .5f) + .5f;
    float s2 = (randomSample.y * .5f) + .5f;
    float s3 = (randomSample.z * .5f) + .5f;

    float x = m_sphere->radius * sqrtf(s1) * cosf(2 * M_PI * s2);
    float y = m_sphere->radius * sqrtf(s1) * sinf(2 * M_PI * s2);
    float z = sqrtf(m_sphere->radius * m_sphere->radius - x * x - y * y) * sinf(M_PI * (s3 - .5f));

    RandomValue<glm::vec3> result;
    result.value = glm::normalize(lightPos + glm::vec3(x, y, z) - m_surfacePoint->position);
    result.probability = 1 / solidAngle(lightPos);
    return result;
}

glm::vec4 SphericalLight::evaluateSample(const glm::vec3& direction) const
{
    return m_emission;
}

float SphericalLight::sampleProbability(const glm::vec3& direction) const
{
    glm::vec3 lightPos(m_sphere->transform * glm::vec4(0, 0, 0, 1));
    return 1 / solidAngle(lightPos);
}
