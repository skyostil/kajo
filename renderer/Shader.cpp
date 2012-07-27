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
                                     const Ray& ray, glm::vec4& color, Random& random) const
{
    std::for_each(objects.begin(), objects.end(), [&](const ObjectType& object){
        if (object.material.emission == glm::vec4(0))
            return;
        if (reinterpret_cast<intptr_t>(&object) == ray.objectId)
            return;
        size_t i = &object - &objects[0];
        applyEmissiveObject(object, transformDataList[i], ray, color, random);
    });
}

void Shader::applyEmissiveObject(const scene::Sphere& sphere, const TransformData& data,
                                 const Ray& ray, glm::vec4& color, Random& random) const
{
    glm::vec3 lightPos(sphere.transform * glm::vec4(0, 0, 0, 1));
    glm::vec3 offset = random.generateSpherical() * sphere.radius;
    glm::vec3 dir(lightPos + offset - ray.hitPos);

    Ray shadowRay;
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

glm::vec4 Shader::shade(const Ray& ray, Random& random, bool indirectLightOnly, int depth) const
{
    if (!ray.hit() || !ray.material)
        return m_scene->backgroundColor;

    glm::vec4 color;
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
                                ray, incomingRadiance, random);
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

    glm::vec4 r = random.generate();
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
        glm::vec3 dir = random.generateCosineHemisphere();
        dir = ray.normal * dir.z + ray.tangent * dir.x + ray.binormal * dir.y;
#else
        glm::vec3 dir = random.generateSpherical();
        if (glm::dot(dir, ray.normal) <= 0)
            dir = -dir;
#endif
        Ray reflectedRay;
        reflectedRay.direction = dir;
        reflectedRay.origin = ray.hitPos + reflectedRay.direction * g_surfaceEpsilon;

        color = glm::vec4(0);
        if (m_raytracer->trace(reflectedRay))
            color += ray.material->diffuse * 1.f / (1 - pEmit) * shade(reflectedRay, random, false, depth + 1);
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
        Ray refractedRay;
        refractedRay.direction = glm::normalize(glm::refract(ray.direction, normal, eta));
        refractedRay.origin = ray.hitPos + refractedRay.direction * g_surfaceEpsilon;

        if (m_raytracer->trace(refractedRay))
            color += shade(refractedRay, random, false, depth + 1) *
                     ray.material->transparency * ray.material->diffuse;
    }

    // Diffuse reflection
    if (!exitingMaterial)
    {
        glm::vec3 dir = random.generateCosineHemisphere();
        dir = ray.normal * dir.z + ray.tangent * dir.x + ray.binormal * dir.y;

        Ray reflectedRay;
        reflectedRay.direction = dir;
        reflectedRay.origin = ray.hitPos + reflectedRay.direction * g_surfaceEpsilon;

        if (m_raytracer->trace(reflectedRay))
            color += shade(reflectedRay, random, true, depth + 1) * ray.material->diffuse;
    }

    // Specular reflection
    if (ray.material->reflectivity && !exitingMaterial)
    {
        Ray reflectedRay;
        reflectedRay.direction = glm::reflect(ray.direction, ray.normal);
        reflectedRay.origin = ray.hitPos + reflectedRay.direction * g_surfaceEpsilon;

        if (m_raytracer->trace(reflectedRay))
            color += shade(reflectedRay, random, false, depth + 1) * ray.material->reflectivity;
    }

    //color = ray.material->color;
    //color = glm::vec4(-ray.normal, 1.f);
    //color = glm::vec4(ray.tangent, 1.f);
    //color = glm::vec4(-ray.binormal, 1.f);
    return color;
#endif
}
