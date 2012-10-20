// Copyright (C) 2012 Sami Kyöstilä

#include "BSDF.h"
#include "Random.h"
#include "Ray.h"
#include "Raytracer.h"
#include "Shader.h"
#include "SurfacePoint.h"
#include "Util.h"
#include "scene/Scene.h"

#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <limits>
#include <cmath>

namespace
{
const float g_surfaceEpsilon = 0.001f;
const int g_depthLimit = 8;
}

Shader::Shader(scene::Scene* scene, Raytracer* raytracer):
    m_scene(scene),
    m_raytracer(raytracer)
{
}

template <typename ObjectType>
glm::vec4 Shader::sampleLights(const std::vector<ObjectType>& objects,
                               const TransformDataList& transformDataList,
                               const SurfacePoint& surfacePoint, BSDF& bsdf,
                               Random& random) const
{
    glm::vec4 color;
    std::for_each(objects.begin(), objects.end(), [&](const ObjectType& object) {
        if (object.material.emission == glm::vec4(0))
            return;
        if (reinterpret_cast<intptr_t>(&object) == surfacePoint.objectId)
            return;
        size_t i = &object - &objects[0];

        // Generate a direction toward the light
        RandomValue<glm::vec3> lightDirection = generateLightSample(object, transformDataList[i], surfacePoint, random);
        if (!lightDirection.probability)
            return;

        // Check for visibility
        Ray shadowRay;
        shadowRay.direction = lightDirection.value;
        shadowRay.origin = surfacePoint.position + shadowRay.direction * g_surfaceEpsilon;
        //shadowRay.maxDistance = glm::length(samplePos - surfacePoint.position); // FIXME
        if (!m_raytracer->canReach(shadowRay, reinterpret_cast<intptr_t>(&object)))
            return;

        // Calculate BSDF probability in the light direction
        float bsdfProbability = bsdf.sampleProbability(lightDirection.value);
        if (bsdfProbability <= 0)
            return;

        color +=
                M_1_PI *
                1 / (bsdfProbability + lightDirection.probability) *
                glm::dot(surfacePoint.normal, lightDirection.value) *
                surfacePoint.material->diffuse *
                object.material.emission;
    });
    return color;
}

template <typename ObjectType>
float Shader::calculateLightProbabilities(const std::vector<ObjectType>& objects,
                                          const TransformDataList& transformDataList,
                                          const SurfacePoint& surfacePoint, const glm::vec3& direction) const
{
    float totalPdf = 0;
    std::for_each(objects.begin(), objects.end(), [&](const ObjectType& object) {
        if (object.material.emission == glm::vec4(0))
            return;
        if (reinterpret_cast<intptr_t>(&object) == surfacePoint.objectId)
            return;
        size_t i = &object - &objects[0];
        totalPdf += calculateLightProbability(object, transformDataList[i], surfacePoint, direction);
    });
    return totalPdf;
}

RandomValue<glm::vec3> Shader::generateLightSample(const scene::Sphere& sphere, const TransformData& data,
                                                   const SurfacePoint& surfacePoint, Random& random) const
{
    glm::vec3 lightPos(sphere.transform * glm::vec4(0, 0, 0, 1));
    glm::vec3 w = lightPos - surfacePoint.position;
    glm::vec3 u = glm::normalize(glm::cross(fabs(w.x) > .1 ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0), w));
    glm::vec3 v = glm::cross(w, u);
    float cos_a_max = sqrtf(1 - sphere.radius * sphere.radius / glm::dot(w, w));
    glm::vec4 randomSample = random.generate();
    float s1 = (randomSample.x * .5f) + .5f;
    float s2 = (randomSample.y * .5f) + .5f;
    float cos_a = 1 - s1 + s1 * cos_a_max;
    float sin_a = sqrtf(1 - cos_a * cos_a);
    float phi = 2 * M_PI * s2;

    RandomValue<glm::vec3> result;
    result.value = glm::normalize(u * cosf(phi) * sin_a + v * sinf(phi) * sin_a + w * cos_a);
    result.probability = 1 / ((1 - cos_a_max) * (2 * M_PI));
    return result;
}

float Shader::calculateLightProbability(const scene::Sphere& sphere, const TransformData& data,
                                        const SurfacePoint& surfacePoint, const glm::vec3& direction) const
{
    glm::vec3 lightPos(sphere.transform * glm::vec4(0, 0, 0, 1));
    glm::vec3 w = lightPos - surfacePoint.position;
    float cos_a_max = sqrtf(1 - sphere.radius * sphere.radius / glm::dot(w, w));
    float cos_a = glm::dot(direction, glm::normalize(w));

    if (cos_a < cos_a_max)
        return 0;
    return 1 / ((1 - cos_a_max) * (2 * M_PI));
}

glm::vec4 Shader::shade(const SurfacePoint& surfacePoint, Random& random, int depth, LightSamplingScheme lightSamplingScheme) const
{
    if (!surfacePoint.valid() || !surfacePoint.material)
        return m_scene->backgroundColor;

    const scene::Material* material = surfacePoint.material;
    LambertBSDF bsdf(&surfacePoint, material->diffuse);

    // Terminate path with Russian roulette
    auto shouldContinue = Random::russianRoulette(random, material->diffuse);
    if (!shouldContinue.value || depth >= g_depthLimit)
        return (1 / shouldContinue.probability) * material->emission;

    // Account for emission
    glm::vec4 color;
    if (lightSamplingScheme == SampleAllObjects)
        color = material->emission;

    // Sample all lights
    color +=
        1 / shouldContinue.probability *
        sampleLights(m_scene->spheres, m_raytracer->precalculatedScene().sphereTransforms, surfacePoint, bsdf, random);

    // Generate new ray direction based on BSDF
    RandomValue<glm::vec3> bsdfDirection = bsdf.generateSample(random);

    Ray ray;
    ray.direction = bsdfDirection.value;
    ray.origin = surfacePoint.position + ray.direction * g_surfaceEpsilon;

    // Trace
    SurfacePoint result = m_raytracer->trace(ray);

    // Calculate light probabilities in the BSDF direction
    float lightProbability =
        calculateLightProbabilities(m_scene->spheres, m_raytracer->precalculatedScene().sphereTransforms,
                                    surfacePoint, bsdfDirection.value);

    // Evaluate BSDF
    color +=
            1 / shouldContinue.probability *
            1 / (lightProbability + bsdfDirection.probability) *
            M_1_PI *
            bsdf.evaluateSample(ray.direction) *
            shade(result, random, depth + 1, SampleNonEmissiveObjects);
    return color;
#if 0
    // If we are only sampling indirect lighting, skip emissive objects. This
    // is done to avoid oversampling emissive objects.
    if (lightSamplingScheme == SampleNonEmissiveObjects && (color.x || color.y || color.z))
        return glm::vec4();

    // Terminate path with Russian roulette
    float invTerminationProbability = russianRoulette(random, material->diffuse);
    if (!invTerminationProbability || depth >= g_depthLimit)
        return color;

    /*
    glm::vec3 direction = surfacePoint.normal;
    color += glm::vec4(1.f) * fabs(pdfForObjects(m_scene->spheres,
             m_raytracer->precalculatedScene().sphereTransforms, surfacePoint,
             direction)) + glm::vec4(surfacePoint.normal.xyzx);*/

    // Sample lights
    color += invTerminationProbability *
        sampleLights(m_scene->spheres, m_raytracer->precalculatedScene().sphereTransforms, surfacePoint, random);

    // Sample BSDF
    // TODO: EvaluateBSDF + recursion
    color += invTerminationProbability * sampleBSDF(surfacePoint, random, depth);

    // FIXME: Is this correct?
    //color /= 2;

    return color;
#endif

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
