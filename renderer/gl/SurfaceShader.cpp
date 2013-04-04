// Copyright (C) 2012 Sami Kyöstilä

#include "SurfaceShader.h"
#include "Scene.h"
#include "Light.h"
#include "Raytracer.h"
#include "Random.h"
#include "BSDF.h"
#include "renderer/GLHelpers.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace gl;

SurfaceShader::SurfaceShader(Scene* scene, Raytracer* raytracer, Random* random):
    m_scene(scene),
    m_raytracer(raytracer),
    m_random(random)
{
}

void SurfaceShader::writeSurfaceShader(std::ostringstream& s) const
{
    m_random->writeRandomNumberGenerator(s);
    m_raytracer->writeRayGenerator(s);
    writeMaterials(s);
    writeLights(s);
    BSDF::writeBSDFFunctions(s);

    s << "void shadeSurfacePoint(inout SurfacePoint surfacePoint,\n"
         "                       vec2 imagePosition, inout vec4 radiance,\n"
         "                       inout vec4 weight)\n"
         "{\n"
         "    Material material = materials[int(surfacePoint.objectIndex)];\n"
         "\n"
         "    float u = 2.0 * random(imagePosition + surfacePoint.position.zy) - 1.0;\n"
         "    float v = random(imagePosition + surfacePoint.position.yx);\n"
         "    float r = sqrt(1.0 - u * u);\n"
         "    float theta = v * 2.0 * M_PI;\n"
         "    vec3 newDirection = vec3(r * cos(theta), r * sin(theta), u);\n"
         "    if (dot(newDirection, surfacePoint.normal) < 0.0)\n"
         "        newDirection = -newDirection;\n"
         "\n"
         "    vec4 newRadiance = radiance + weight * material.emission;\n"
         "    newRadiance += weight * (material.specular + material.diffuse) * sampleLights(surfacePoint.position, surfacePoint.normal) * 0.01;\n"
         "    newRadiance.w = radiance.w;\n"
         "\n"
         "    vec4 newWeight = weight * (material.specular + material.diffuse) * max(0.0, dot(newDirection, surfacePoint.normal));\n"
         "    float maxWeight = max(newWeight.x, max(newWeight.y, newWeight.z));\n"
         "    if (maxWeight < 0.01) {\n"
         "        generateRay(imagePosition, surfacePoint.position, newDirection);\n"
         "        newWeight = vec4(1.0);\n"
         "        newRadiance.w = newRadiance.w + 1.0;\n"
         "    }\n"
         "\n"
         "    surfacePoint.position += newDirection * 0.001;\n"
         "    surfacePoint.view = newDirection;\n"
         "    radiance = newRadiance;\n"
         "    weight = newWeight;\n"
         "}\n"
         "\n";
}

void SurfaceShader::writeMaterials(std::ostringstream& s) const
{
    size_t objectCount = m_scene->spheres.size() + m_scene->planes.size();
    size_t materialIndex = 0;

    s << "struct Material {\n"
         "    vec4 ambient;\n"
         "    vec4 diffuse;\n"
         "    vec4 specular;\n"
         "    vec4 emission;\n"
         "    vec4 transparency;\n"
         "    float specularExponent;\n"
         "    float refractiveIndex;\n"
         "};\n"
         "\n";

    s << "Material materials[" << objectCount << "] = Material[" << objectCount << "](\n";
    for (size_t i = 0; i < m_scene->planes.size(); i++) {
        s << "    ";
        m_scene->planes[i].material.writeInitializer(s);
        if (++materialIndex != objectCount)
            s << ",";
        s << "\n";
    }
    for (size_t i = 0; i < m_scene->spheres.size(); i++) {
        s << "    ";
        m_scene->spheres[i].material.writeInitializer(s);
        if (++materialIndex != objectCount)
            s << ",";
        s << "\n";
    }
    s << ");\n"
         "\n";
}

void SurfaceShader::writeLights(std::ostringstream& s) const
{
    SphericalLight::writeCommon(s);
    s << "\n";

    for (size_t i = 0; i < m_scene->spheres.size(); i++) {
        if (m_scene->spheres[i].material.material.emission == glm::vec4())
            continue;

        SphericalLight light(&m_scene->spheres[i]);
        size_t objectIndex = m_scene->objectIndex(m_scene->spheres[i]);

        std::string name = "generateLight" + std::to_string(objectIndex) + "Sample";
        light.writeSampleGenerator(s, name);
        s << "\n";

        name = "evaluateLight" + std::to_string(objectIndex) + "Sample";
        light.writeSampleEvaluator(s, name);
        s << "\n";

        name = "light" + std::to_string(objectIndex) + "SampleProbability";
        light.writeSampleProbability(s, name);
        s << "\n";
    }

    s << "vec4 sampleLights(vec3 surfacePos, vec3 normal)\n"
         "{\n"
         "    vec4 radiance = vec4(0.0);\n";

    for (size_t i = 0; i < m_scene->spheres.size(); i++) {
        if (m_scene->spheres[i].material.material.emission == glm::vec4())
            continue;

        size_t objectIndex = m_scene->objectIndex(m_scene->spheres[i]);
        s << "    {\n"
             "        vec3 direction;\n"
             "        float lightProbability;\n"
             "        generateLight" << objectIndex << "Sample(surfacePos, direction, lightProbability);\n"
             "        if (lightProbability > 0.0) {\n"
             "            vec3 origin = surfacePos + direction * 0.001;\n"
             "            if (rayCanReach(origin, direction, " << objectIndex << ")) {\n"
             "                float bsdfProbability = 0.0;\n"
             "                radiance += \n"
             "                    1.0 / (bsdfProbability + lightProbability) *\n"
             "                    /* bsdf todo */\n"
             "                    max(0.0, dot(normal, direction)) *\n"
             "                    evaluateLight" << objectIndex << "Sample(surfacePos, direction);\n"
             "            }\n"
             "        }\n"
             "    }\n";
    }
    s << "    return radiance;\n"
         "}\n"
         "\n";
}

void SurfaceShader::setSurfaceShaderUniforms(GLuint program) const
{
    m_random->setRandomNumberGeneratorUniforms(program);
    m_raytracer->setRayGeneratorUniforms(program);
}
