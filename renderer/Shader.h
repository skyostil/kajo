// Copyright (C) 2012 Sami Kyöstilä
#ifndef SHADER_H
#define SHADER_H

#include "PrecalculatedScene.h"

#include <glm/glm.hpp>
#include <memory>
#include <vector>

class Ray;
class Raytracer;
class Surface;

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

    glm::vec4 shade(const Ray& ray, int depth = 0) const;

private:
    template <typename LightType>
    void applyAllLights(const std::vector<LightType>& lights,
                        const TransformDataList& transformDataList,
                        const Ray& ray, glm::vec4& color) const;

    float lightOcclusion(const Ray& ray, const scene::PointLight& light,
                         const TransformData& data) const;

    void applyLight(const Ray& ray, glm::vec4& color, float occlusion,
                    const scene::PointLight& light,
                    const TransformData& data) const;

    scene::Scene* m_scene;
    Raytracer* m_raytracer;

    int m_depthLimit;
};

#endif
