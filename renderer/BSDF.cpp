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
    return m_color * glm::dot(m_surfacePoint->normal, direction);
}

float LambertBSDF::sampleProbability(const glm::vec3& direction) const
{
    // Uniform BSDF over the hemisphere.
    float hemisphereArea = 4 * M_PI / 2;
    if (glm::dot(direction, m_surfacePoint->normal) <= 0)
        return 0;
    return 1.f / hemisphereArea;
}
