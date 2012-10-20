// Copyright (C) 2012 Sami Kyöstilä
#ifndef SHADER_H
#define SHADER_H

#include "PrecalculatedScene.h"
#include "Random.h"
#include "Ray.h"

#include <glm/glm.hpp>
#include <memory>
#include <vector>

class BSDF;
class Random;
class Raytracer;
class SurfacePoint;

namespace scene
{
    class Material;
    class Sphere;
    class Plane;
    class PointLight;
    class Scene;
}

class Shader
{
public:
    Shader(scene::Scene* scene, Raytracer* raytracer);

    enum LightSamplingScheme
    {
        SampleNonEmissiveObjects,
        SampleAllObjects,
    };

    glm::vec4 shade(const SurfacePoint& surfacePoint, Random& random, int depth = 0, LightSamplingScheme = SampleAllObjects) const;

private:
    template <typename ObjectType>
    glm::vec4 sampleLights(const std::vector<ObjectType>& lights,
                           const TransformDataList& transformDataList,
                           const SurfacePoint&, BSDF&, Random&) const;
    RandomValue<glm::vec3> generateLightSample(const scene::Sphere& sphere, const TransformData& data,
                                               const SurfacePoint& surfacePoint, Random& random) const;

    template <typename ObjectType>
    float calculateLightProbabilities(const std::vector<ObjectType>& lights,
                                      const TransformDataList& transformDataList,
                                      const SurfacePoint&, const glm::vec3& direction) const;
    float calculateLightProbability(const scene::Sphere& sphere, const TransformData& data,
                                    const SurfacePoint&, const glm::vec3& direction) const;

    scene::Scene* m_scene;
    Raytracer* m_raytracer;
};

#endif
