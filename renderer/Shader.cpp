// Copyright (C) 2012 Sami Kyöstilä

#include "Shader.h"
#include "scene/Scene.h"
#include "Random.h"
#include "Ray.h"
#include "Raytracer.h"
#include "SurfacePoint.h"
#include "Util.h"

#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <limits>
#include <cmath>

namespace
{
const float g_surfaceEpsilon = 0.001f;
const float g_randomEpsilon = 1e-3f;
//const float g_bsdfWeight = 1;//.5f;
//const float g_lightWeight = 1;//1.f - g_bsdfWeight;
const float g_bsdfWeight = 1;
const float g_lightWeight = 1;
const int g_depthLimit = 8;
}

Shader::Shader(scene::Scene* scene, Raytracer* raytracer):
    m_scene(scene),
    m_raytracer(raytracer)
{
}

/*
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
                                 const SurfacePoint& surfacePoint, glm::vec4& color) const
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
*/

template <typename ObjectType>
glm::vec4 Shader::sampleLights(const std::vector<ObjectType>& objects,
                               const TransformDataList& transformDataList,
                               const SurfacePoint& surfacePoint, Random& random) const
{
    glm::vec4 color;
    std::for_each(objects.begin(), objects.end(), [&](const ObjectType& object) {
        if (object.material.emission == glm::vec4(0))
            return;
        if (reinterpret_cast<intptr_t>(&object) == surfacePoint.objectId)
            return;
        size_t i = &object - &objects[0];

        // Generate a direction toward the light.
        RandomValue<glm::vec3> lightDirection = generateLightSample(object, transformDataList[i], surfacePoint, random);
        //Sample lightSample(random);
        //generateLightSample(lightSample, object, transformDataList[i], surfacePoint);
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
        float bsdfProbability = calculateBSDFProbability(surfacePoint, lightDirection.value);
        if (bsdfProbability <= 0)
            return;

        // Evaluate the sample
        //glm::vec3 lightValue = evaluateLightSample(sphere, transformDataList[i], surfacePoint, lightDirection.value);

        // TODO: Probability for just this single light
        //float lightProbabilities =
            //calculateLightProbabilities(m_scene->spheres, m_raytracer->precalculatedScene().sphereTransforms,
                                        //surfacePoint, lightSample.ray.direction);
        // FIXME: Is this right for multiple lights? A: No
        //float weight = 1.f / (g_bsdfWeight * bsdfProbability + g_lightWeight * lightProbabilities);
        color +=
                M_1_PI *
                1 / (bsdfProbability + lightDirection.probability) *
                glm::dot(surfacePoint.normal, lightDirection.value) *
                surfacePoint.material->diffuse *
                object.material.emission;
        //printf("%f\n", weight);
        //dump(lightSample);
        //if (lightProbability < 0.0 || lightProbability > 1000)
            //printf("%f\n", weight / lightProbability);
        //if (weight != std::numeric_limits<float>::infinity())
        //color += lightSample;
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
/*
    glm::vec4 randomSample = random.generate();
    float s1 = (randomSample.x * .5f) + .5f;
    float s2 = (randomSample.y * .5f) + .5f;
    float s3 = (randomSample.z * .5f);

    glm::vec3 lightPos(sphere.transform * glm::vec4(0, 0, 0, 1));
    glm::vec3 dir(lightPos - surfacePoint.position);

    glm::vec3 samplePos;
    samplePos.x = sphere.radius * sqrtf(s1) * cosf(2 * M_PI * s2);
    samplePos.y = sphere.radius * sqrtf(s1) * sinf(2 * M_PI * s2),
    samplePos.z =
        sqrtf(fabs(sphere.radius * sphere.radius - samplePos.x * samplePos.x - samplePos.y * samplePos.y)) *
            sinf(M_PI * s3);
    samplePos += lightPos;

    return RandomValue<glm::vec3>(glm::normalize(samplePos - surfacePoint.position));
*/
}

#if 0
glm::vec3 Shader::evaluateLightSample(const scene::Sphere& sphere, const TransformData& data,
                                      const SurfacePoint& surfacePoint, const glm::vec3& direction) const
    // Check for visibility
    Ray shadowRay;
    shadowRay.direction = direction;
    shadowRay.origin = surfacePoint.position + shadowRay.direction * g_surfaceEpsilon;
    shadowRay.maxDistance = glm::length(samplePos - surfacePoint.position);

    if (!m_raytracer->canReach(shadowRay, reinterpret_cast<intptr_t>(&sphere)))
        return glm::vec3();
/*
    Ray shadowRay;
    shadowRay.direction = sample.ray.direction;
    shadowRay.origin = surfacePoint.position + shadowRay.direction * g_surfaceEpsilon;
    shadowRay.maxDistance = glm::length(samplePos - surfacePoint.position);

    if (m_raytracer->canReach(shadowRay, reinterpret_cast<intptr_t>(&sphere)))
        sample.value =
            surfacePoint.material->diffuse *
            sphere.material.emission *
            fabs(glm::dot(surfacePoint.normal, sample.ray.direction));
#if 0

    glm::vec3 lightPos(sphere.transform * glm::vec4(0, 0, 0, 1));
    glm::vec3 dir(lightPos - surfacePoint.position);
    glm::vec3 w = glm::normalize(lightPos - surfacePoint.position);
    glm::vec3 v = glm::normalize(glm::cross(w, surfacePoint.normal));
    glm::vec3 u = glm::cross(w, v);
    float r = sphere.radius - g_surfaceEpsilon; // Make sure we always hit the light.
    float d = r * r / glm::dot(dir, dir);
    float cos_alpha = 1 - s1 + s1 * sqrtf(1 - d);
    float sin_alpha = sqrtf(1 - cos_alpha * cos_alpha);
    float phi = 2 * M_PI * s2;

    sample.ray.direction = glm::vec3(cosf(phi) * sin_alpha, sinf(phi) * sin_alpha, cos_alpha);
    sample.ray.direction = glm::vec3(
            glm::dot(glm::vec3(u.x, v.x, w.x), sample.ray.direction),
            glm::dot(glm::vec3(u.y, v.y, w.y), sample.ray.direction),
            glm::dot(glm::vec3(u.z, v.z, w.z), sample.ray.direction));

    Ray shadowRay;
    shadowRay.direction = sample.ray.direction;
    shadowRay.origin = surfacePoint.position + shadowRay.direction * g_surfaceEpsilon;
    shadowRay.maxDistance = glm::length(dir) + sphere.radius; // Is this right?

    if (m_raytracer->canReach(shadowRay, reinterpret_cast<intptr_t>(&sphere)))
        sample.value = surfacePoint.material->diffuse * sphere.material.emission * glm::dot(surfacePoint.normal, sample.ray.direction);
#endif
*/
}
#endif

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
/*
    glm::vec3 lightPos(sphere.transform * glm::vec4(0, 0, 0, 1));
    glm::vec3 dir(lightPos - surfacePoint.position);

    if (glm::dot(dir, surfacePoint.normal) <= 0)
        return 0;

    Ray lightRay;
    lightRay.direction = direction;
    lightRay.origin = surfacePoint.position + lightRay.direction * g_surfaceEpsilon;

    SurfacePoint lightSurfacePoint;
    m_raytracer->intersect(lightRay, lightSurfacePoint, sphere, data);

    if (!lightSurfacePoint.valid())
        return 0;

    float dist = dir.length();
    float solidAngle;
    if (dist > sphere.radius)
       solidAngle = 2 * M_PI * (1 - cosf(asinf(sphere.radius / dist)));
    else
       solidAngle = 4 * M_PI;

    return 1 / solidAngle;
*/
#if 0
    float d = (sphere.radius * sphere.radius) / glm::dot(dir, dir);
    float max_cos_theta = sqrtf(1 - d);

    /*{
        float cos_theta = glm::dot(dir, direction);
        if (cos_theta > max_cos_theta)
            return 0;
    }*/

    // Hmm, do we really need to trace here?
    Ray lightRay;
    lightRay.direction = direction;
    lightRay.origin = surfacePoint.position + lightRay.direction * g_surfaceEpsilon;

    SurfacePoint lightSurfacePoint;
    m_raytracer->intersect(lightRay, lightSurfacePoint, sphere, data);

    if (!lightSurfacePoint.valid())
        return 0;

    //printf("%f: %d\n", lightRay.maxDistance, lightRay.hit());

    lightSurfacePoint.position = lightRay.origin + lightRay.direction * lightRay.maxDistance;
    float cos_theta = glm::dot(-direction, lightSurfacePoint.normal);
    float p = cos_theta /
        (2 * M_PI * glm::dot(lightSurfacePoint.position - surfacePoint.position, lightSurfacePoint.position - surfacePoint.position) * (1 - max_cos_theta));

    //if (p <= 0.0 || p > 1000)
    //    printf("%f\n", p);

    return p;
#endif
}

#if 0
void Shader::generateBSDFSample(Sample& sample, const SurfacePoint& surfacePoint, int depth) const
{
    sample.ray.direction = sample.random.generateSpherical();
    if (glm::dot(sample.ray.direction, surfacePoint.normal) < 0)
        sample.ray.direction = -sample.ray.direction;
    sample.ray.origin = surfacePoint.position + sample.ray.direction * g_surfaceEpsilon;

    SurfacePoint result = m_raytracer->trace(sample.ray);
    if (!result.valid())
        sample.value = m_scene->backgroundColor;
    else
    {
        sample.value = surfacePoint.material->diffuse *
            glm::dot(surfacePoint.normal, sample.ray.direction) *
            shade(result, sample.random, depth + 1, SampleNonEmissiveObjects);
    }
}
#endif

float Shader::calculateBSDFProbability(const SurfacePoint& surfacePoint, const glm::vec3& direction) const
{
    // Uniform BSDF over the hemisphere.
    float hemisphereArea = 4 * M_PI / 2;
    if (glm::dot(direction, surfacePoint.normal) <= 0)
        return 0;
    // FIXME: Is this right?
    //return .5f;
    return 1.f / hemisphereArea;
}

#if 0
glm::vec4 Shader::sampleBSDF(const SurfacePoint& surfacePoint, Random& random, int depth) const
{
    Sample bsdfSample(random);
    generateBSDFSample(bsdfSample, surfacePoint, depth);
    if (bsdfSample.value == glm::vec4())
        return bsdfSample.value;

    float bsdfProbability = calculateBSDFProbability(surfacePoint, bsdfSample.ray.direction);
    return bsdfSample.value / bsdfProbability;
/*
    float lightProbability =
        calculateLightProbabilities(m_scene->spheres, m_raytracer->precalculatedScene().sphereTransforms,
                                    surfacePoint, bsdfSample.ray.direction);
    if (lightProbability <= 0)
        return color;

    float bsdfProbability = calculateBSDFProbability(surfacePoint, bsdfSample.ray.direction);
    float weight = 1.f / (g_bsdfWeight * bsdfProbability + g_lightWeight * lightProbability);
    //if (weight != std::numeric_limits<float>::infinity())
        color += bsdfSample.value * weight;
    return color;
*/
}
#endif

static RandomValue<bool> russianRoulette(Random& random, const glm::vec4& probability)
{
    // Note: w component ignored
    float p = std::max(probability.x, std::max(probability.y, probability.z));
    float r = random.generate().x * .5f + .5f;
    if (r <= p)
        return RandomValue<bool>(true, p);
    return RandomValue<bool>(false, 1 - p);
}

class Sample
{
public:
    Sample(Random& random):
        random(random),
        pdf(0)
    {
    }

    Random& random;
    //Ray ray;
    glm::vec3 direction;
    //glm::vec4 value;
    float pdf;
};

class BSDF
{
public:
    BSDF(const glm::vec4& color);

    void generateSample(const SurfacePoint& surfacePoint, Random& random, glm::vec3& direction, glm::vec4& value, float& pdf) const;
    void evaluateSample(const SurfacePoint& surfacePoint, const glm::vec3& direction, glm::vec4& value, float& pdf) const;

private:
    glm::vec4 m_color;
};

BSDF::BSDF(const glm::vec4& color):
    m_color(color)
{
}

#if 0
void BSDF::generateSample(const SurfacePoint& surfacePoint, Random& random,
                          glm::vec3& direction, glm::vec4& value, float& pdf) const
{
    // TODO: Cosine hemisphere distribution
    direction = random.generateSpherical();
    if (glm::dot(direction, surfacePoint.normal) < 0)
        direction = -direction;

    float hemisphereArea = 4 * M_PI / 2;
    value = m_color * fabs(glm::dot(surfacePoint.normal, direction));
    pdf = 1 / hemisphereArea;
}
#endif

void BSDF::evaluateSample(const SurfacePoint& surfacePoint, const glm::vec3& direction,
                          glm::vec4& value, float& pdf) const
{
    if (glm::dot(direction, surfacePoint.normal) < 0)
        return;

    float hemisphereArea = 4 * M_PI / 2;
    value = m_color * fabs(glm::dot(surfacePoint.normal, direction));
    pdf = 1 / hemisphereArea;
}

glm::vec4 Shader::shade(const SurfacePoint& surfacePoint, Random& random, int depth, LightSamplingScheme lightSamplingScheme) const
{
    if (!surfacePoint.valid() || !surfacePoint.material)
        return m_scene->backgroundColor;

    const scene::Material* material = surfacePoint.material;

    // Terminate path with Russian roulette
    auto shouldContinue = russianRoulette(random, material->diffuse);
    if (!shouldContinue.value || depth >= g_depthLimit)
        return (1 / shouldContinue.probability) * material->emission;

    // Account for emission
    glm::vec4 color;
    if (lightSamplingScheme == SampleAllObjects)
        color = material->emission;

    // Sample all lights
    color +=
        1 / shouldContinue.probability *
        sampleLights(m_scene->spheres, m_raytracer->precalculatedScene().sphereTransforms, surfacePoint, random);

    // Generate new ray direction based on BSDF
    //RandomValue<glm::vec3> bsdfDirection(random.generateSpherical());
    RandomValue<glm::vec3> bsdfDirection(random.generateHemispherical(surfacePoint.normal));
    //RandomValue<glm::vec3> bsdfDirection(random.generateCosineHemispherical(surfacePoint.normal, surfacePoint.tangent, surfacePoint.binormal));

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
            material->diffuse *
            glm::dot(surfacePoint.normal, ray.direction) *
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
