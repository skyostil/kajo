// Copyright (C) 2012 Sami Kyöstilä
#ifndef RAYTRACER_H
#define RAYTRACER_H

#include "PrecalculatedScene.h"

#include <glm/glm.hpp>
#include <memory>
#include <vector>

class Ray;
class Shader;
class Surface;

namespace scene
{
    class Material;
    class Sphere;
    class Plane;
    class PointLight;
    class Scene;
}

class Raytracer
{
public:
    Raytracer(scene::Scene* scene);

    bool trace(Ray& ray) const;
    const PrecalculatedScene& precalculatedScene() const;

private:
    template <typename ObjectType>
    void intersectAll(const std::vector<ObjectType>& objects,
                      const TransformDataList& transformDataList,
                      Ray& ray) const;

    void intersect(Ray& ray, const scene::Sphere& sphere, const TransformData& data) const;
    void intersect(Ray& ray, const scene::Plane& plane, const TransformData& data) const;

    void processIntersection(Ray& ray, float t, intptr_t objectId,
                             const glm::vec3& normal, const glm::vec3& tangent,
                             const glm::vec3& binormal,
                             const scene::Material* material) const;

    scene::Scene* m_scene;
    std::unique_ptr<PrecalculatedScene> m_precalcScene;
};

#endif
