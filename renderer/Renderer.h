// Copyright (C) 2012 Sami Kyöstilä
#ifndef RENDERER_H
#define RENDERER_H

#include <glm/glm.hpp>
#include <vector>

class Ray;
class Surface;

namespace scene
{
    class Material;
    class Sphere;
    class Plane;
    class PointLight;
    class Scene;
}

class TransformData
{
public:
    glm::mat4 invTransform;
    float determinant;
};

typedef std::vector<TransformData> TransformDataList;

class PrecalculatedScene
{
public:
    TransformDataList sphereTransforms;
    TransformDataList planeTransforms;
    TransformDataList pointLightTransforms;
};

class Renderer
{
public:
    Renderer(scene::Scene* scene);

    void render(Surface& surface, int xOffset, int yOffset, int width, int height) const;

    bool trace(Ray& ray) const;
    glm::vec4 sample(const Ray& ray) const;

private:
    void prepare();

    template <typename ObjectType>
    void prepareAll(std::vector<ObjectType>& objects, TransformDataList& transformDataList);

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

    void applyLight(const Ray& ray, glm::vec4& color, const scene::PointLight& light,
                    const TransformData& data) const;

    scene::Scene* m_scene;
    PrecalculatedScene m_precalcScene;
};

#endif
