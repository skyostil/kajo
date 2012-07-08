// Copyright (C) 2012 Sami Kyöstilä

#include "Shader.h"
#include "scene/Scene.h"
#include "Ray.h"
#include "Raytracer.h"

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
    m_depthLimit(4)
{
}

template <typename LightType>
void Shader::applyAllLights(const std::vector<LightType>& lights,
                            const TransformDataList& transformDataList,
                            const Ray& ray, glm::vec4& color) const
{
    std::for_each(lights.begin(), lights.end(), [&](const LightType& light){
        size_t i = &light - &lights[0];
        float occlusion = lightOcclusion(ray, light, transformDataList[i]);
        applyLight(ray, color, occlusion, light, transformDataList[i]);
    });
}

float Shader::lightOcclusion(const Ray& ray, const scene::PointLight& light,
                             const TransformData& data) const
{
    glm::vec3 lightPos(light.transform * glm::vec4(0, 0, 0, 1));
    glm::vec3 dir(lightPos - ray.hitPos);

    Ray shadowRay;
    shadowRay.direction = glm::normalize(dir);
    shadowRay.origin = ray.hitPos + shadowRay.direction * g_surfaceEpsilon;
    shadowRay.maxDistance = glm::length(dir);

    if (!m_raytracer->trace(shadowRay))
        return 1;

    return 0;
}

void Shader::applyLight(const Ray& ray, glm::vec4& color, float occlusion,
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

glm::vec4 Shader::shade(const Ray& ray, int depth) const
{
    if (!ray.hit || !ray.material)
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
        applyAllLights(m_scene->pointLights, m_raytracer->precalculatedScene().pointLightTransforms, ray, color);
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

        if (m_raytracer->trace(refractedRay))
            color += shade(refractedRay, depth + 1) * ray.material->transparency;
    }

    // Reflection
    if (ray.material->reflectivity && !internalIntersection && depth < m_depthLimit)
    {
        Ray reflectedRay;
        reflectedRay.direction = glm::reflect(ray.direction, ray.normal);
        reflectedRay.origin = ray.hitPos + reflectedRay.direction * g_surfaceEpsilon;

        if (m_raytracer->trace(reflectedRay))
            color += shade(reflectedRay, depth + 1) * ray.material->reflectivity;
    }

    //color = ray.material->color;
    //color = glm::vec4(-ray.normal, 1.f);
    return color;
}
