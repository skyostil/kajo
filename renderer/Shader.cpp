// Copyright (C) 2012 Sami Kyöstilä

#include "Shader.h"
#include "scene/Scene.h"
#include "Random.h"
#include "Ray.h"
#include "Raytracer.h"
#include "Util.h"

#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <limits>
#include <cmath>

namespace
{
const float g_surfaceEpsilon = 0.001f;
}

Shader::Shader(scene::Scene* scene, Raytracer* raytracer):
    m_scene(scene),
    m_raytracer(raytracer),
    m_depthLimit(10)
{
}

template <typename ObjectType>
void Shader::applyAllEmissiveObjects(const std::vector<ObjectType>& objects,
                                     const TransformDataList& transformDataList,
                                     const Ray& ray, glm::vec4& color) const
{
    std::for_each(objects.begin(), objects.end(), [&](const ObjectType& object){
        if (object.material.emission == glm::vec4(0))
            return;
        if (reinterpret_cast<intptr_t>(&object) == ray.objectId)
            return;
        size_t i = &object - &objects[0];
        applyEmissiveObject(object, transformDataList[i], ray, color);
    });
}

void Shader::applyEmissiveObject(const scene::Sphere& sphere, const TransformData& data,
                                 const Ray& ray, glm::vec4& color) const
{
    glm::vec3 lightPos(sphere.transform * glm::vec4(0, 0, 0, 1));
    glm::vec3 offset = ray.random.generateSpherical() * sphere.radius;
    glm::vec3 dir(lightPos + offset - ray.hitPos);

    Ray shadowRay(ray.random);
    shadowRay.direction = glm::normalize(dir);
    shadowRay.origin = ray.hitPos + shadowRay.direction * g_surfaceEpsilon;
    shadowRay.maxDistance = glm::length(dir);

    if (!m_raytracer->trace(shadowRay))
        return;

    if (shadowRay.objectId != reinterpret_cast<intptr_t>(&sphere))
        return;

    glm::vec3 distance = lightPos - ray.hitPos;
    glm::vec3 normal = glm::dot(ray.direction, ray.normal) < 0 ? ray.normal : -ray.normal; // FIXME: precalc
    float c = sqrt(1.f - sphere.radius * sphere.radius / glm::dot(distance, distance));
    float o = 2 * M_PI * (1 - c);
    color += sphere.material.emission * glm::dot(shadowRay.direction, normal) * o * M_1_PI;
}

template <typename ObjectType>
glm::vec4 Shader::sampleObjects(const std::vector<ObjectType>& objects,
                                const TransformDataList& transformDataList,
                                const Ray& ray) const
{
    glm::vec4 color;
    std::for_each(objects.begin(), objects.end(), [&](const ObjectType& object) {
        if (object.material.emission == glm::vec4(0))
            return;
        if (reinterpret_cast<intptr_t>(&object) == ray.objectId)
            return;
        size_t i = &object - &objects[0];
        glm::vec3 direction;
        glm::vec4 lightSample = sampleObject(object, transformDataList[i], ray, direction);
        float lightProbability = pdfForObject(object, transformDataList[i], ray, direction);
        float brdfProbability = pdfForBRDF(ray, direction);
        float totalLightProbability =
            pdfForObjects(m_scene->spheres, m_raytracer->precalculatedScene().sphereTransforms,
                          ray, direction);
        float weight = lightProbability / (brdfProbability + totalLightProbability);
        color += lightSample * weight / lightProbability;
    });
    return color;
}

template <typename ObjectType>
float Shader::pdfForObjects(const std::vector<ObjectType>& objects,
                            const TransformDataList& transformDataList,
                            const Ray& ray, const glm::vec3& direction) const
{
    float totalPdf = 0;
    std::for_each(objects.begin(), objects.end(), [&](const ObjectType& object) {
        if (object.material.emission == glm::vec4(0))
            return;
        if (reinterpret_cast<intptr_t>(&object) == ray.objectId)
            return;
        size_t i = &object - &objects[0];
        totalPdf += pdfForObject(object, transformDataList[i], ray, direction);
    });
    return totalPdf;
}

glm::vec4 Shader::sampleObject(const scene::Sphere& sphere, const TransformData& data,
                               const Ray& ray, glm::vec3& direction) const
{
    glm::vec4 randomSample = ray.random.generate();
    float s1 = (randomSample.x * .5f) + .5f;
    float s2 = (randomSample.y * .5f) + .5f;

    glm::vec3 lightPos(sphere.transform * glm::vec4(0, 0, 0, 1));
    glm::vec3 dir(lightPos - ray.hitPos);
    glm::vec3 w = glm::normalize(lightPos - ray.hitPos);
    glm::vec3 v = glm::normalize(glm::cross(w, ray.normal));
    glm::vec3 u = glm::cross(w, v);
    float d = sphere.radius * sphere.radius / glm::dot(dir, dir);
    float cos_alpha = 1 - s1 + s1 * sqrtf(1 - d);
    float sin_alpha = sqrtf(1 - cos_alpha * cos_alpha);
    float phi = 2 * M_PI * s2;

    direction = glm::vec3(cosf(phi) * sin_alpha, sin(phi) * sin_alpha, cos_alpha);
    direction = glm::vec3(
            glm::dot(glm::vec3(u.x, v.x, w.x), direction),
            glm::dot(glm::vec3(u.y, v.y, w.y), direction),
            glm::dot(glm::vec3(u.z, v.z, w.z), direction));

    Ray shadowRay(ray.random);
    shadowRay.direction = glm::normalize(direction);
    shadowRay.origin = ray.hitPos + shadowRay.direction * g_surfaceEpsilon;
    shadowRay.maxDistance = glm::length(dir) + sphere.radius; // Is this right?

    if (!m_raytracer->trace(shadowRay))
        return glm::vec4();

    if (shadowRay.objectId != reinterpret_cast<intptr_t>(&sphere))
        return glm::vec4();

    return sphere.material.emission;
}

float Shader::pdfForObject(const scene::Sphere& sphere, const TransformData& data,
                           const Ray& ray, const glm::vec3& direction) const
{
    glm::vec3 lightPos(sphere.transform * glm::vec4(0, 0, 0, 1));
    glm::vec3 dir(glm::normalize(lightPos - ray.hitPos));
    float cos_theta = glm::dot(dir, direction);
    float d = sphere.radius * sphere.radius / glm::dot(dir, dir);
    float max_cos_theta = sqrtf(1 - d);
    if (cos_theta > max_cos_theta)
        return 0;

    Ray lightRay(ray.random);
    lightRay.direction = direction;
    lightRay.origin = ray.hitPos + lightRay.direction * g_surfaceEpsilon;
    m_raytracer->intersect(lightRay, sphere, data);

    glm::vec3 x = ray.origin + ray.direction * ray.maxDistance;
    float p = glm::dot(-direction, ray.normal) /
        (2 * M_PI * glm::dot(x - ray.hitPos, x - ray.hitPos) * (1 - max_cos_theta));
    return p;
}

glm::vec4 Shader::sampleBRDF(const Ray& ray, glm::vec3& direction, int depth) const
{
    direction = ray.random.generateSpherical();
    if (glm::dot(direction, ray.normal) <= 0)
        direction = -direction;

    Ray brdfRay(ray.random);
    brdfRay.direction = direction;
    brdfRay.origin = ray.hitPos + brdfRay.direction * g_surfaceEpsilon;
    return shade(brdfRay, depth + 1);
}

float Shader::pdfForBRDF(const Ray& ray, const glm::vec3& direction) const
{
#if 1
    // Uniform BRDF over the hemisphere.
    float hemisphereArea = 4 * M_PI / 2;
    if (glm::dot(direction, ray.normal) <= 0)
        return 0.f;
    return 1.f / hemisphereArea;
#else
    // Uniform BRDF over the entire sphere.
    return 1.f / (4 * M_PI);
#endif
}

glm::vec4 Shader::shade(const Ray& ray, int depth) const
{
    if (!ray.hit() || !ray.material)
        return m_scene->backgroundColor;

    glm::vec4 color = ray.material->emission;

    if (depth >= m_depthLimit)
        return color;

    // Sample BRDF
    {
        glm::vec3 direction;
        glm::vec4 brdfSample = sampleBRDF(ray, direction, depth);
        float brdfProbability = pdfForBRDF(ray, direction);
        float lightProbability =
            pdfForObjects(m_scene->spheres, m_raytracer->precalculatedScene().sphereTransforms,
                          ray, direction);
        float weight = brdfProbability / (brdfProbability + lightProbability);
        color += brdfSample * weight / brdfProbability;
    }

    // Sample lights
    color += sampleObjects(m_scene->spheres, m_raytracer->precalculatedScene().sphereTransforms,
                           ray);

    return color;

#if 0
    glm::vec3 dir = ray.random.generateSpherical();
    if (glm::dot(dir, ray.normal) <= 0)
        dir = -dir;

    Ray reflectedRay(ray.random);
    reflectedRay.direction = dir;
    reflectedRay.origin = ray.hitPos + reflectedRay.direction * g_surfaceEpsilon;

    if (m_raytracer->trace(reflectedRay))
        color += ray.material->diffuse * shade(reflectedRay, false, depth + 1);

    return color;
#endif

#if 0
    bool exitingMaterial = (glm::dot(ray.direction, ray.normal) >= 0);

    //if (!indirectLightOnly)
        color = ray.material->emission;

    if (depth >= m_depthLimit)
        return glm::vec4(0);

#if 0
    if (!exitingMaterial)
    {
        glm::vec4 incomingRadiance;
        applyAllEmissiveObjects(m_scene->spheres, m_raytracer->precalculatedScene().sphereTransforms,
                                ray, incomingRadiance);
        color += incomingRadiance * ray.material->diffuse;
    }
    else
        color = glm::vec4(0);
#endif

    if (ray.material->checkerboard)
    {
        if ((fmod(fmod(ray.hitPos.x, 1) + 1, 1) < 0.5f) ^ (fmod(fmod(ray.hitPos.z, 1) + 1, 1) < .5f))
            color *= .5f;
    }

    float pEmit = (glm::dot(color, color) > .01f) ? .9f : 0.f;

    glm::vec4 r = ray.random.generate();
    r.x = .5f * r.x + .5f;
    if (r.x < pEmit)
    {
        // Emitted
        return 1.f / pEmit * color;
    }
    else
    {
        // Reflected
#if 0
        glm::vec3 dir = ray.random.generateCosineHemisphere();
        dir = ray.normal * dir.z + ray.tangent * dir.x + ray.binormal * dir.y;
#else
        glm::vec3 dir = ray.random.generateSpherical();
        if (glm::dot(dir, ray.normal) <= 0)
            dir = -dir;
#endif
        Ray reflectedRay(ray.random);
        reflectedRay.direction = dir;
        reflectedRay.origin = ray.hitPos + reflectedRay.direction * g_surfaceEpsilon;

        color = glm::vec4(0);
        if (m_raytracer->trace(reflectedRay))
            color += ray.material->diffuse * 1.f / (1 - pEmit) * shade(reflectedRay, false, depth + 1);
        //dump(color);
        return color;
    }

#if 0
    // Refraction
    if (ray.material->transparency)
    {
        // Intersecting transparent objects not supported
        float eta = 1.f / ray.material->refractiveIndex;
        glm::vec3 normal = ray.normal;
        if (exitingMaterial)
        {
            // Leaving the material
            eta = 1.f / eta;
            normal = -normal;
        }
        Ray refractedRay(ray.random);
        refractedRay.direction = glm::normalize(glm::refract(ray.direction, normal, eta));
        refractedRay.origin = ray.hitPos + refractedRay.direction * g_surfaceEpsilon;

        if (m_raytracer->trace(refractedRay))
            color += shade(refractedRay, false, depth + 1) *
                     ray.material->transparency * ray.material->diffuse;
    }

    // Diffuse reflection
    if (!exitingMaterial)
    {
        glm::vec3 dir = ray.random.generateCosineHemisphere();
        dir = ray.normal * dir.z + ray.tangent * dir.x + ray.binormal * dir.y;

        Ray reflectedRay(ray.random);
        reflectedRay.direction = dir;
        reflectedRay.origin = ray.hitPos + reflectedRay.direction * g_surfaceEpsilon;

        if (m_raytracer->trace(reflectedRay))
            color += shade(reflectedRay, true, depth + 1) * ray.material->diffuse;
    }

    // Specular reflection
    if (ray.material->reflectivity && !exitingMaterial)
    {
        Ray reflectedRay(ray.random);
        reflectedRay.direction = glm::reflect(ray.direction, ray.normal);
        reflectedRay.origin = ray.hitPos + reflectedRay.direction * g_surfaceEpsilon;

        if (m_raytracer->trace(reflectedRay))
            color += shade(reflectedRay, false, depth + 1) * ray.material->reflectivity;
    }

    //color = ray.material->color;
    //color = glm::vec4(-ray.normal, 1.f);
    //color = glm::vec4(ray.tangent, 1.f);
    //color = glm::vec4(-ray.binormal, 1.f);
    return color;
#endif
#endif
}
