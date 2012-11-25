// Copyright (C) 2012 Sami Kyöstilä
#ifndef CPU_LIGHT_H
#define CPU_LIGHT_H

#include <glm/glm.hpp>

#include "Scene.h"

namespace cpu
{

class Sphere;
class Random;
class Raytracer;
class SurfacePoint;

template <typename T> class RandomValue;

class Light
{
public:
    Light(const SurfacePoint*, const Raytracer*);

    virtual RandomValue<glm::vec3> generateSample(Random& random) const = 0;
    virtual glm::vec4 evaluateSample(const glm::vec3& direction) const = 0;
    virtual float sampleProbability(const glm::vec3& direction) const = 0;

protected:
    const SurfacePoint* m_surfacePoint;
    const Raytracer* m_raytracer;
};

class SphericalLight: public Light
{
public:
    SphericalLight(const SurfacePoint*, const Raytracer*, const Sphere*, const glm::vec4& emission);

    RandomValue<glm::vec3> generateSample(Random& random) const override;
    glm::vec4 evaluateSample(const glm::vec3& direction) const override;
    float sampleProbability(const glm::vec3& direction) const override;

private:
    float solidAngle(const glm::vec3& lightPos) const;

    const Sphere* m_sphere;
    glm::vec4 m_emission;
};

}

#endif // LIGHT_H
