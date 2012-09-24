// Copyright (C) 2012 Sami Kyöstilä
#ifndef SHADER_H
#define SHADER_H

#include "PrecalculatedScene.h"
#include "Ray.h"

#include <glm/glm.hpp>
#include <memory>
#include <vector>

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
    class Sample
    {
    public:
        Sample(Random& random):
            random(random)
        {
        }

        Random& random;
        Ray ray;
        glm::vec4 value;
    };

    glm::vec4 sampleBSDF(const SurfacePoint&, Random&, int depth) const;
    void generateBSDFSample(Sample&, const SurfacePoint&, int depth) const;
    float calculateBSDFProbability(const SurfacePoint&, const glm::vec3& direction) const;

    template <typename ObjectType>
    glm::vec4 sampleLights(const std::vector<ObjectType>& lights,
                           const TransformDataList& transformDataList,
                           const SurfacePoint&, Random&) const;
    void generateLightSample(Sample&, const scene::Sphere& sphere, const TransformData& data,
                             const SurfacePoint&) const;


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
