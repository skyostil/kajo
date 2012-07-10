// Copyright (C) 2012 Sami Kyöstilä

#include "Raytracer.h"
#include "Ray.h"
#include "Surface.h"
#include "Util.h"

#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <limits>
#include <cmath>

Raytracer::Raytracer(scene::Scene* scene):
    m_scene(scene),
    m_precalcScene(new PrecalculatedScene(scene))
{
}

const PrecalculatedScene& Raytracer::precalculatedScene() const
{
    return *m_precalcScene.get();
}

void Raytracer::intersect(Ray& ray, const scene::Sphere& sphere, const TransformData& data) const
{
    // From http://wiki.cgsociety.org/index.php/Ray_Sphere_Intersection
    glm::vec3 dir = glm::mat3(data.invTransform) * ray.direction;
    glm::vec3 origin = ((data.invTransform * glm::vec4(ray.origin, 1.f))).xyz();
    float a = glm::dot(dir, dir);
    float b = 2 * glm::dot(dir, origin);
    float c = glm::dot(origin, origin) - sphere.radius * sphere.radius;

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

    if (t0 > t1)
        std::swap(t0, t1);

    if (t1 < 0)
        return;

    if (t0 < 0)
        t0 = t1;

    glm::vec3 normal = origin + dir * t0;
    normal = glm::normalize(glm::mat3(sphere.transform) * normal);
    intptr_t objectId = reinterpret_cast<intptr_t>(&sphere);
#if 0
    glm::vec3 tangent;
    float smallest = std::min(normal.z, std::min(normal.x, normal.y));
    if (normal.x == smallest)
        tangent = glm::vec3(0, -normal.z, normal.y);
    else if (normal.y == smallest)
        tangent = glm::vec3(-normal.z, 0, normal.x);
    else
        tangent = glm::vec3(-normal.y, normal.x, 0);
    tangent = glm::normalize(tangent);
#endif
    glm::vec3 tangent = glm::cross(normal, glm::vec3(0.f, 1.f, 0.f));
    glm::vec3 binormal = glm::cross(normal, tangent);

    processIntersection(ray, t0 * data.determinant, objectId, normal,
                        tangent, binormal, &sphere.material);
}

void Raytracer::intersect(Ray& ray, const scene::Plane& plane, const TransformData& data) const
{
    glm::vec3 dir = glm::mat3(data.invTransform) * ray.direction;
    glm::vec3 origin = ((data.invTransform * glm::vec4(ray.origin, 1.f))).xyz();
    glm::vec3 normal(0, 1.f, 0);

    float denom = glm::dot(dir, normal);

    if (fabs(denom) < std::numeric_limits<float>::epsilon())
        return;

    float t = -glm::dot(origin, normal) / denom;

    if (t < 0)
        return;

    normal = glm::mat3(plane.transform) * -normal;
    glm::vec3 tangent = glm::mat3(plane.transform) * glm::vec3(1, 0, 0);
    glm::vec3 binormal = glm::cross(normal, tangent);

    intptr_t objectId = reinterpret_cast<intptr_t>(&plane);

    processIntersection(ray, t * data.determinant, objectId, normal,
                        tangent, binormal, &plane.material);
}

template <typename ObjectType>
void Raytracer::intersectAll(const std::vector<ObjectType>& objects,
                            const TransformDataList& transformDataList,
                            Ray& ray) const
{
    std::for_each(objects.begin(), objects.end(), [&](const ObjectType& object){
        size_t i = &object - &objects[0];
        intersect(ray, object, transformDataList[i]);
    });
}

void Raytracer::processIntersection(Ray& ray, float t,
                                    intptr_t objectId,
                                    const glm::vec3& normal,
                                    const glm::vec3& tangent,
                                    const glm::vec3& binormal,
                                    const scene::Material* material) const
{
    if (t > ray.maxDistance || t < ray.minDistance)
        return;

    ray.objectId = objectId;
    ray.maxDistance = t;
    ray.normal = normal;
    ray.tangent = tangent;
    ray.binormal = binormal;
    ray.material = material;
}

bool Raytracer::trace(Ray& ray) const
{
    intersectAll(m_scene->planes, m_precalcScene->planeTransforms, ray);
    intersectAll(m_scene->spheres, m_precalcScene->sphereTransforms, ray);

    if (!ray.hit())
        return false;

    ray.hitPos = ray.origin + ray.direction * ray.maxDistance;
    return true;
}
