// Copyright (C) 2012 Sami Kyöstilä
#ifndef LIGHT_H
#define LIGHT_H

#include <glm/glm.hpp>

class Random;
class SurfacePoint;
class TransformData;

namespace scene
{
class Sphere;
}

template <typename T> class RandomValue;

class Light
{
public:
    Light(const SurfacePoint*);

    virtual RandomValue<glm::vec3> generateSample(Random& random) const = 0;
    virtual glm::vec4 evaluateSample(const glm::vec3& direction) const = 0;
    virtual float sampleProbability(const glm::vec3& direction) const = 0;

protected:
    const SurfacePoint* m_surfacePoint;
};

class SphericalLight: public Light
{
public:
    SphericalLight(const SurfacePoint*, const scene::Sphere*, const TransformData*, const glm::vec4& emission);

    RandomValue<glm::vec3> generateSample(Random& random) const override;
    glm::vec4 evaluateSample(const glm::vec3& direction) const override;
    float sampleProbability(const glm::vec3& direction) const override;

private:
    const scene::Sphere* m_sphere;
    const TransformData* m_transformData;
    glm::vec4 m_emission;
};

#endif // LIGHT_H
