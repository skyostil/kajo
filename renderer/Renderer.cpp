// Copyright (C) 2012 Sami Kyöstilä

#include "Ray.h"
#include "Renderer.h"
#include "Surface.h"
#include "Util.h"

#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <limits>
#include <cmath>

namespace
{
const float g_surfaceEpsilon = 0.001f;
}

Renderer::Renderer(scene::Scene* scene):
    m_scene(scene),
    m_depthLimit(4)
{
    prepare();
}

template <typename ObjectType>
void Renderer::prepareAll(std::vector<ObjectType>& objects, TransformDataList& transformDataList)
{
    std::for_each(objects.begin(), objects.end(), [&](ObjectType& object){
        TransformData transformData;
        transformData.invTransform = glm::inverse(object.transform);
        transformData.determinant = glm::determinant(object.transform);
        transformDataList.push_back(transformData);
    });
}

void Renderer::prepare()
{
    prepareAll(m_scene->spheres, m_precalcScene.sphereTransforms);
    prepareAll(m_scene->planes, m_precalcScene.planeTransforms);
    prepareAll(m_scene->pointLights, m_precalcScene.pointLightTransforms);
}

void Renderer::intersect(Ray& ray, const scene::Sphere& sphere, const TransformData& data) const
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

    processIntersection(ray, t0 * data.determinant, normal, &sphere.material);
}

void Renderer::intersect(Ray& ray, const scene::Plane& plane, const TransformData& data) const
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

    processIntersection(ray, t * data.determinant, normal, &plane.material);
}

template <typename ObjectType>
void Renderer::intersectAll(const std::vector<ObjectType>& objects,
                            const TransformDataList& transformDataList,
                            Ray& ray) const
{
    std::for_each(objects.begin(), objects.end(), [&](const ObjectType& object){
        size_t i = &object - &objects[0];
        intersect(ray, object, transformDataList[i]);
    });
}

void Renderer::processIntersection(Ray& ray, float t,
                                   const glm::vec3& normal,
                                   const scene::Material* material) const
{
    if (t > ray.nearest)
        return;

    ray.nearest = t;
    ray.normal = normal;
    ray.material = material;
}

bool Renderer::trace(Ray& ray) const
{
    intersectAll(m_scene->planes, m_precalcScene.planeTransforms, ray);
    intersectAll(m_scene->spheres, m_precalcScene.sphereTransforms, ray);

    if (!ray.hit())
        return false;

    ray.hitPos = ray.origin + ray.direction * ray.nearest;
    return true;
}

template <typename LightType>
void Renderer::applyAllLights(const std::vector<LightType>& lights,
                              const TransformDataList& transformDataList,
                              const Ray& ray, glm::vec4& color) const
{
    std::for_each(lights.begin(), lights.end(), [&](const LightType& light){
        size_t i = &light - &lights[0];
        float occlusion = lightOcclusion(ray, light, transformDataList[i]);
        applyLight(ray, color, occlusion, light, transformDataList[i]);
    });
}

float Renderer::lightOcclusion(const Ray& ray, const scene::PointLight& light,
                               const TransformData& data) const
{
    glm::vec3 lightPos(light.transform * glm::vec4(0, 0, 0, 1));
    glm::vec3 dir(lightPos - ray.hitPos);
    float lightDist = glm::dot(dir, dir);

    Ray shadowRay;
    shadowRay.direction = glm::normalize(dir);
    shadowRay.origin = ray.hitPos + shadowRay.direction * g_surfaceEpsilon;

    if (!trace(shadowRay))
        return 1;

    return (shadowRay.nearest * shadowRay.nearest < lightDist) ? 0 : 1;
}

void Renderer::applyLight(const Ray& ray, glm::vec4& color, float occlusion,
                          const scene::PointLight& light,
                          const TransformData& data) const
{
    glm::vec3 lightPos(light.transform * glm::vec4(0, 0, 0, 1));
    glm::vec3 dir(lightPos - ray.hitPos);

    float invDistance = 1.f / glm::dot(dir, dir);
    dir = glm::normalize(dir);

    // Ambient
    color += ray.material->ambient * light.color;

    // Diffuse
    float intensity = glm::clamp(glm::dot(dir, ray.normal), 0.f, 1.f) * light.intensity;
    color += intensity * ray.material->diffuse * invDistance * light.color * occlusion;

    // Specular
    if (ray.material->specularExponent)
    {
        glm::vec3 reflection = glm::reflect(dir, ray.normal);
        intensity = glm::pow(glm::clamp(glm::dot(ray.direction, reflection), 0.f, 1.f),
                             ray.material->specularExponent);
        intensity *= light.intensity;
        color += intensity * light.color * invDistance * occlusion;
    }
}

glm::vec4 Renderer::sample(const Ray& ray, int depth) const
{
    if (!ray.hit() || !ray.material)
        return m_scene->backgroundColor;

    bool internalIntersection = (glm::dot(ray.direction, ray.normal) >= 0);

    glm::vec4 color = ray.material->ambient;
    color.a = 1.f;

    if (ray.material->checkerboard)
    {
        if ((fmod(fmod(ray.hitPos.x, 1) + 1, 1) < 0.5f) ^ (fmod(fmod(ray.hitPos.z, 1) + 1, 1) < 0.5f))
            color.rgb = glm::vec3(color.rgb) * 4.f;
    }

    if (!internalIntersection)
        applyAllLights(m_scene->pointLights, m_precalcScene.pointLightTransforms, ray, color);
    else
        color.rgb = glm::vec3(0);

    // Refraction
    if (ray.material->transparency && depth < m_depthLimit)
    {
        // Intersecting transparent objects not supported
        float eta = 1.f / ray.material->refractiveIndex;
        glm::vec3 normal = ray.normal;
        if (internalIntersection)
        {
            // Leaving the material
            eta = 1.f / eta;
            normal = -normal;
        }
        Ray refractedRay;
        refractedRay.direction = glm::normalize(glm::refract(ray.direction, normal, eta));
        refractedRay.origin = ray.hitPos + refractedRay.direction * g_surfaceEpsilon;

        if (trace(refractedRay))
            color += sample(refractedRay, depth + 1) * ray.material->transparency;
    }

    // Reflection
    if (ray.material->reflectivity && !internalIntersection && depth < m_depthLimit)
    {
        Ray reflectedRay;
        reflectedRay.direction = glm::reflect(ray.direction, ray.normal);
        reflectedRay.origin = ray.hitPos + reflectedRay.direction * g_surfaceEpsilon;

        if (trace(reflectedRay))
            color += sample(reflectedRay, depth + 1) * ray.material->reflectivity;
    }

    //color = ray.material->color;
    //color = glm::vec4(-ray.normal, 1.f);
    return color;
}

void Renderer::render(Surface& surface, int xOffset, int yOffset, int width, int height) const
{
    const scene::Camera& camera = m_scene->camera;

    const glm::vec4 viewport(0, 0, 1, 1);
    glm::vec3 p1 = glm::unProject(glm::vec3(0.f, 0.f, 0.f), camera.transform, camera.projection, viewport);
    glm::vec3 p2 = glm::unProject(glm::vec3(1.f, 0.f, 0.f), camera.transform, camera.projection, viewport);
    glm::vec3 p3 = glm::unProject(glm::vec3(0.f, 1.f, 0.f), camera.transform, camera.projection, viewport);
    glm::vec3 origin(glm::inverse(camera.transform) * glm::vec4(0.f, 0.f, 0.f, 1.f));

    for (int y = yOffset; y < yOffset + height; y++)
    {
        for (int x = xOffset; x < xOffset + width; x++)
        {
            float wx = static_cast<float>(x) / surface.width;
            float wy = 1 - static_cast<float>(y) / surface.height;
            glm::vec3 direction = p1 + (p2 - p1) * wx + (p3 - p1) * wy - origin;
            direction = glm::normalize(direction);

            Ray ray;
            ray.origin = origin;
            ray.direction = direction;

            trace(ray);
            glm::vec4 color = sample(ray);

            surface.pixels[y * surface.width + x] = Surface::colorToRGBA8(color);
        }
    }
}
