// Copyright (C) 2012 Sami Kyöstilä
#ifndef SHADER_H
#define SHADER_H

#include "PrecalculatedScene.h"

#include <glm/glm.hpp>
#include <memory>
#include <vector>

class Random;
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

    glm::vec4 shade(const Ray& ray, Random& random, bool indirectLightOnly = false, int depth = 0) const;

private:
    template <typename ObjectType>
    void applyAllEmissiveObjects(const std::vector<ObjectType>& lights,
                                 const TransformDataList& transformDataList,
                                 const Ray& ray, glm::vec4& color, Random& random) const;

    void applyEmissiveObject(const scene::Sphere& sphere, const TransformData& data,
                             const Ray& ray, glm::vec4& color, Random& random) const;

    scene::Scene* m_scene;
    Raytracer* m_raytracer;

    int m_depthLimit;
};

#endif
