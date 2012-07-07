// Copyright (C) 2012 Sami Kyöstilä

#include "Renderer.h"
#include "Surface.h"
#include "Util.h"

#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <limits>
#include <cmath>

Renderer::Renderer(scene::Scene* scene):
    m_scene(scene)
{
}

class Ray
{
public:
    Ray();

    bool hit() const;

    glm::vec3 origin;
    glm::vec3 direction;
    
    // Result
    float nearest;
    glm::vec3 normal;
    const scene::Material* material;
};

Ray::Ray():
    nearest(std::numeric_limits<float>::infinity()),
    material(0)
{
}

bool Ray::hit() const
{
    return nearest != std::numeric_limits<float>::infinity();
}

void Renderer::intersect(Ray& ray, const scene::Sphere& sphere) const
{
    // From http://wiki.cgsociety.org/index.php/Ray_Sphere_Intersection
    glm::vec3 dir = glm::mat3(sphere.transform) * ray.direction;
    glm::vec3 origin = ((sphere.transform * glm::vec4(ray.origin, 1.f))).xyz();
    float a = glm::dot(dir, dir);
    float b = 2 * glm::dot(dir, origin);
    float c = glm::dot(origin, origin) - sphere.radius * sphere.radius;

    //dump(origin);
    //dump(dir);
    //printf("a %f b %f c %f\n", a, b, c);

    float discr = b * b - 4 * a * c;
    if (discr < 0)
        return;

    float q;
    if (b < 0)
        q = (-b - sqrtf(discr)) * .5f;
    else
        q = (-b + sqrtf(discr)) * .5f;

    float t0 = q / a;
    float t1 = c / q;

    //printf("a %f b %f c %f\n", a, b, c);

    if (t0 > t1)
        std::swap(t0, t1);

    //printf("%f..%f\n", t0, t1);

    if (t1 < 0)
        return;

    if (t0 < 0)
        processIntersection(ray, t1, glm::normalize(origin + dir * t1), &sphere.material);
    else
        processIntersection(ray, t0, glm::normalize(origin + dir * t0), &sphere.material);
}

void Renderer::intersect(Ray& ray, const scene::Plane& plane) const
{
}

template <typename ObjectType>
void Renderer::intersectAll(const std::vector<ObjectType>& objects, Ray& ray) const
{
    std::for_each(objects.begin(), objects.end(), [&](const ObjectType& object){
        intersect(ray, object);
    });
}

void Renderer::processIntersection(Ray& ray, float t, const glm::vec3& normal,
                                   const scene::Material* material) const
{
    if (t > ray.nearest)
        return;

    ray.nearest = t;
    ray.normal = normal;
    ray.material = material;
}

glm::vec4 Renderer::sample(const Ray& ray) const
{
    glm::vec4 color = m_scene->backgroundColor;
    if (!ray.hit() || !ray.material)
        return color;

    color = ray.material->color;
    color = glm::vec4(-ray.normal, 1.f);
    return color;
}

void Renderer::render(Surface& surface, int xOffset, int yOffset, int width, int height) const
{
    const scene::Camera& camera = m_scene->camera;
    const glm::vec3 origin(camera.view * glm::vec4(0.f, 0.f, 0.f, 1.f));

    const glm::vec4 viewport(0, 0, 1, 1);
    const glm::vec3 p1 = glm::unProject(glm::vec3(0.f, 0.f, 0.f), camera.view, camera.projection, viewport);
    const glm::vec3 p2 = glm::unProject(glm::vec3(1.f, 0.f, 0.f), camera.view, camera.projection, viewport);
    const glm::vec3 p3 = glm::unProject(glm::vec3(0.f, 1.f, 0.f), camera.view, camera.projection, viewport);

    for (int y = yOffset; y < yOffset + height; y++)
    {
        for (int x = xOffset; x < xOffset + width; x++)
        {
            float wx = static_cast<float>(x) / surface.width;
            float wy = static_cast<float>(y) / surface.height;
            glm::vec3 direction = p1 + (p2 - p1) * wx + (p3 - p1) * wy - origin;
            direction = glm::normalize(direction);

            Ray ray;
            ray.origin = origin;
            ray.direction = direction;
            intersectAll(m_scene->spheres, ray);
            intersectAll(m_scene->planes, ray);

            glm::vec4 color = sample(ray);
            surface.pixels[y * surface.width + x] = Surface::colorToRGBA8(color);
        }
    }
}
