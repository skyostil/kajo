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

    void processIntersection(Ray& ray, float t, const glm::vec3& normal,
                             const scene::Material* material) const;

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
    std::unique_ptr<PrecalculatedScene> m_precalcScene;
};

#endif
