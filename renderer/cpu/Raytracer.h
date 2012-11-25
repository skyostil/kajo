// Copyright (C) 2012 Sami Kyöstilä
#ifndef CPU_RAYTRACER_H
#define CPU_RAYTRACER_H

#include "Scene.h"

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

    void intersect(Ray&, SurfacePoint&, const Sphere&) const;
    void intersect(Ray&, SurfacePoint&, const Plane&) const;

private:
    template <typename ObjectType>
    void intersectAll(const std::vector<ObjectType>& objects,
                      Ray&, SurfacePoint&) const;

    void processIntersection(Ray&, SurfacePoint&, float t, intptr_t objectId,
                             const glm::vec3& normal, const glm::vec3& tangent,
                             const glm::vec3& binormal,
                             const Material* material) const;

    Scene* m_scene;
};

}

#endif
