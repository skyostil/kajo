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
class SurfacePoint;

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

    SurfacePoint trace(Ray&) const;
    bool canReach(Ray&, intptr_t objectId) const;

    const PrecalculatedScene& precalculatedScene() const;

    void intersect(Ray&, SurfacePoint&, const scene::Sphere&, const TransformData&) const;
    void intersect(Ray&, SurfacePoint&, const scene::Plane&, const TransformData&) const;

private:
    template <typename ObjectType>
    void intersectAll(const std::vector<ObjectType>& objects,
                      const TransformDataList& transformDataList,
                      Ray&, SurfacePoint&) const;

    void processIntersection(Ray&, SurfacePoint&, float t, intptr_t objectId,
                             const glm::vec3& normal, const glm::vec3& tangent,
                             const glm::vec3& binormal,
                             const scene::Material* material) const;

    scene::Scene* m_scene;
    std::unique_ptr<PrecalculatedScene> m_precalcScene;
};

#endif
