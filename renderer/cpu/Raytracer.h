// Copyright (C) 2012 Sami Kyöstilä
#ifndef RAYTRACER_H
#define RAYTRACER_H

#include "PrecalculatedScene.h"

#include <glm/glm.hpp>
#include <memory>
#include <stdint.h>
#include <vector>

namespace cpu
{

class Ray;
class Shader;
class SurfacePoint;
class Material;
class Sphere;
class Plane;
class PointLight;
class Scene;

class Raytracer
{
public:
    Raytracer(Scene* scene);

    SurfacePoint trace(Ray&) const;
    bool canReach(Ray&, intptr_t objectId) const;

    const PrecalculatedScene& precalculatedScene() const;

    void intersect(Ray&, SurfacePoint&, const Sphere&, const TransformData&) const;
    void intersect(Ray&, SurfacePoint&, const Plane&, const TransformData&) const;

private:
    template <typename ObjectType>
    void intersectAll(const std::vector<ObjectType>& objects,
                      const TransformDataList& transformDataList,
                      Ray&, SurfacePoint&) const;

    void processIntersection(Ray&, SurfacePoint&, float t, intptr_t objectId,
                             const glm::vec3& normal, const glm::vec3& tangent,
                             const glm::vec3& binormal,
                             const Material* material) const;

    Scene* m_scene;
    std::unique_ptr<PrecalculatedScene> m_precalcScene;
};

}

#endif
