// Copyright (C) 2012 Sami Kyöstilä
#ifndef CPU_SHADER_H
#define CPU_SHADER_H

#include "Scene.h"
#include "Random.h"
#include "Ray.h"

#include <glm/glm.hpp>
#include <memory>
#include <vector>

namespace cpu
{

class Material;
class Sphere;
class Plane;
class PointLight;
class Scene;
class BSDF;
class Random;
class Raytracer;
class SurfacePoint;

class Shader
{
public:
    Shader(Scene* scene, Raytracer* raytracer);

    enum LightSamplingScheme
    {
        SampleNonEmissiveObjects,
        SampleAllObjects,
    };

    glm::vec4 shade(const SurfacePoint&, Random&, int depth = 0, LightSamplingScheme = SampleAllObjects) const;

private:
    glm::vec4 shadeWithBSDF(const BSDF&, const SurfacePoint&, Random&, int depth, LightSamplingScheme) const;

    template <typename ObjectType>
    glm::vec4 sampleLights(const std::vector<ObjectType>& lights,
                           const SurfacePoint&, const BSDF&, Random&) const;

    template <typename ObjectType>
    float calculateLightProbabilities(const std::vector<ObjectType>& lights,
                                      const SurfacePoint&, const glm::vec3& direction) const;

    Scene* m_scene;
    Raytracer* m_raytracer;
};

}

#endif
