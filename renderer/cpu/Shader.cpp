// Copyright (C) 2012 Sami Kyöstilä

#include "renderer/Util.h"
#include "scene/Scene.h"
#include "BSDF.h"
#include "Light.h"
#include "Random.h"
#include "Ray.h"
#include "Raytracer.h"
#include "Shader.h"
#include "SurfacePoint.h"
#include "Scene.h"

#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <limits>
#include <cmath>

using namespace cpu;

namespace
{
const float g_surfaceEpsilon = 0.001f;
const int g_depthLimit = 8;
}

Shader::Shader(Scene* scene, Raytracer* raytracer):
    m_scene(scene),
    m_raytracer(raytracer)
{
}

template <typename ObjectType>
class LightSampler
{
};

template <>
class LightSampler<Sphere>
{
public:
    LightSampler(const SurfacePoint* surfacePoint, const Raytracer* raytracer, const Sphere* sphere):
        light(surfacePoint, raytracer, sphere, sphere->material.emission)
    {
    }

    SphericalLight light;
};

template <typename ObjectType>
glm::vec4 Shader::sampleLights(const std::vector<ObjectType>& objects,
                               const SurfacePoint& surfacePoint, const BSDF& bsdf,
                               Random& random) const
{
    glm::vec4 radiance;
    for (const ObjectType& object: objects) {
        if (object.material.emission == glm::vec4(0))
            continue;
        if (reinterpret_cast<intptr_t>(&object) == surfacePoint.objectId)
            continue;

        LightSampler<ObjectType> sampler(&surfacePoint, m_raytracer, &object);
        RandomValue<glm::vec3> lightDirection = sampler.light.generateSample(random);
        if (!lightDirection.probability)
            continue;

        // Check for visibility
        Ray shadowRay;
        shadowRay.direction = lightDirection.value;
        shadowRay.origin = surfacePoint.position + shadowRay.direction * g_surfaceEpsilon;
        //shadowRay.maxDistance = glm::length(samplePos - surfacePoint.position); // FIXME
        if (!m_raytracer->canReach(shadowRay, reinterpret_cast<intptr_t>(&object)))
            continue;

        // Calculate BSDF probability in the light direction
        float bsdfProbability = bsdf.sampleProbability(lightDirection.value);
        if (!bsdfProbability)
            continue;

        radiance += 1 / (bsdfProbability + lightDirection.probability) *
                    bsdf.evaluateSample(lightDirection.value) *
                    std::max(0.f, glm::dot(surfacePoint.normal, lightDirection.value)) *
                    sampler.light.evaluateSample(lightDirection.value);
    }
    return radiance;
}

template <typename ObjectType>
float Shader::calculateLightProbabilities(const std::vector<ObjectType>& objects,
                                          const SurfacePoint& surfacePoint, const glm::vec3& direction) const
{
    float totalPdf = 0;
    for (const ObjectType& object: objects) {
        if (object.material.emission == glm::vec4(0))
            continue;
        if (reinterpret_cast<intptr_t>(&object) == surfacePoint.objectId)
            continue;

        // Check for visibility
        Ray shadowRay;
        shadowRay.direction = direction;
        shadowRay.origin = surfacePoint.position + shadowRay.direction * g_surfaceEpsilon;
        //shadowRay.maxDistance = glm::length(samplePos - surfacePoint.position); // FIXME
        if (!m_raytracer->canReach(shadowRay, reinterpret_cast<intptr_t>(&object)))
            continue;

        LightSampler<ObjectType> sampler(&surfacePoint, m_raytracer, &object);
        totalPdf += sampler.light.sampleProbability(direction);
    }
    return totalPdf;
}

glm::vec4 Shader::shade(const SurfacePoint& surfacePoint, Random& random, int depth,
                        LightSamplingScheme lightSamplingScheme) const
{
    if (!surfacePoint.valid() || !surfacePoint.material)
        return m_scene->backgroundColor;

    // Account for emission
    const Material* material = surfacePoint.material;
    glm::vec4 radiance = (lightSamplingScheme == SampleAllObjects) ? material->emission : glm::vec4();

    // Terminate path with Russian roulette
    auto shouldContinue = random.russianRoulette(
            glm::max(glm::max(material->diffuse, material->specular), material->transparency));
    if (!shouldContinue.value || depth >= g_depthLimit)
        return 1 / shouldContinue.probability * radiance;

    // Choose BSDF to be used
    float totalDiffuse = material->diffuse.x + material->diffuse.y + material->diffuse.z;
    float totalSpecular = material->specular.x + material->specular.y + material->specular.z;
    float totalTransparency = material->transparency.x + material->transparency.y + material->transparency.z;
    float transparencyProbability = totalTransparency / (totalDiffuse + totalSpecular + totalTransparency);
    auto transparentSample = random.flipCoin(transparencyProbability);

    // Should this be a transparent sample?
    if (transparentSample.value) {
        float cos_a = glm::dot(surfacePoint.view, surfacePoint.normal);
        bool enteringMaterial = (cos_a < 0);
        glm::vec3 normal = enteringMaterial ? surfacePoint.normal : -surfacePoint.normal;
        float airRefractiveIndex = 1;
        float eta = enteringMaterial ? airRefractiveIndex / material->refractiveIndex :
                                       material->refractiveIndex / airRefractiveIndex;
        cos_a = glm::dot(surfacePoint.view, normal);

        // Total internal reflection
        Ray transmittedRay;
        if (1 - eta * eta * (1 - cos_a * cos_a) < 0) {
            transmittedRay.direction = glm::reflect(surfacePoint.view, normal);
        } else {
            transmittedRay.direction = glm::refract(surfacePoint.view, normal, eta);
        }
        transmittedRay.origin = surfacePoint.position + transmittedRay.direction * g_surfaceEpsilon;
        SurfacePoint result = m_raytracer->trace(transmittedRay);

        return 1 / shouldContinue.probability *
               1 / transparentSample.probability *
               (radiance + material->transparency * shade(result, random, depth + 1, lightSamplingScheme));
    }

    float diffuseProbability = totalDiffuse / (totalDiffuse + totalSpecular);
    auto diffuseSample = random.flipCoin(diffuseProbability);

    // Shade using the BSDF
    if (!diffuseSample.value) {
        if (material->specularExponent) {
            PhongBSDF bsdf(&surfacePoint, material->specular, material->specularExponent);
            return 1 / shouldContinue.probability *
                   1 / transparentSample.probability *
                   1 / diffuseSample.probability *
                   (radiance + shadeWithBSDF(bsdf, surfacePoint, random, depth, lightSamplingScheme));
        } else {
            IdealReflectorBSDF bsdf(&surfacePoint, material->specular);
            return 1 / shouldContinue.probability *
                   1 / transparentSample.probability *
                   1 / diffuseSample.probability *
                   (radiance + shadeWithBSDF(bsdf, surfacePoint, random, depth, lightSamplingScheme));
        }
    }

    LambertBSDF bsdf(&surfacePoint, material->diffuse);
    return 1 / shouldContinue.probability *
           1 / transparentSample.probability *
           1 / diffuseSample.probability *
           (radiance + shadeWithBSDF(bsdf, surfacePoint, random, depth, lightSamplingScheme));
}

glm::vec4 Shader::shadeWithBSDF(const BSDF& bsdf, const SurfacePoint& surfacePoint, Random& random,
                                int depth, LightSamplingScheme lightSamplingScheme) const
{
    const bool directLighting = true;
    glm::vec4 radiance;

    // Sample all lights
    if (directLighting)
        radiance += sampleLights(m_scene->spheres,
                                 surfacePoint, bsdf, random);

    // Generate new ray direction based on BSDF
    RandomValue<glm::vec3> bsdfDirection = bsdf.generateSample(random);
    if (!bsdfDirection.probability)
        return radiance;

    // Trace
    Ray ray;
    ray.direction = bsdfDirection.value;
    ray.origin = surfacePoint.position + ray.direction * g_surfaceEpsilon;
    SurfacePoint result = m_raytracer->trace(ray);

    // Calculate light probabilities in the BSDF direction
    float lightProbability =
        directLighting ? calculateLightProbabilities(m_scene->spheres,
                                                     surfacePoint, bsdfDirection.value) : 0;

    // Evaluate BSDF
    radiance +=
            1 / (lightProbability + bsdfDirection.probability) *
            bsdf.evaluateSample(ray.direction) *
            std::max(0.f, glm::dot(surfacePoint.normal, ray.direction)) *
            shade(result, random, depth + 1, directLighting ? SampleNonEmissiveObjects : SampleAllObjects);

    return radiance;
}
