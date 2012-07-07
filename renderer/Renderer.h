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
    class Scene;
}

class Renderer
{
public:
    Renderer(scene::Scene* scene);

    void render(Surface& surface, int xOffset, int yOffset, int width, int height) const;

private:
    template <typename ObjectType>
    void intersectAll(const std::vector<ObjectType>& objects, Ray& ray) const;

    void intersect(Ray& ray, const scene::Sphere& sphere) const;
    void intersect(Ray& ray, const scene::Plane& plane) const;

    void processIntersection(Ray& ray, float t, const glm::vec3& normal,
                             const scene::Material* material) const;
    glm::vec4 sample(const Ray& ray) const;

    scene::Scene* m_scene;
};

#endif
