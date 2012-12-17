// Copyright (C) 2012 Sami Kyöstilä

#include "SurfaceShader.h"
#include "Scene.h"
#include "Raytracer.h"
#include "Random.h"
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
    s << "#version 120\n" // for first class arrays
         "#define M_PI 3.1415926535897932384626433832795\n"
         "\n"
         "struct Material {\n"
         "    vec4 ambient;\n"
         "    vec4 diffuse;\n"
         "    vec4 specular;\n"
         "    vec4 emission;\n"
         "    vec4 transparency;\n"
         "    float specularExponent;\n"
         "    float refractiveIndex;\n"
         "};\n"
         "\n";

    m_random->writeRandomNumberGenerator(s);
    m_raytracer->writeRayGenerator(s);

    size_t objectCount = m_scene->spheres.size() + m_scene->planes.size();
    size_t materialIndex = 0;

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

    s << "void shadeSurfacePoint(inout vec3 origin, inout vec3 direction, float distance, vec3 normal,\n"
         "                       int objectIndex, vec2 imagePosition, inout vec4 radiance, inout vec4 weight)\n"
         "{\n"
         "    Material material = materials[objectIndex];\n"
         "\n"
         "    vec3 newOrigin = origin + direction * distance;\n"
         "\n"
         "    float u = 2.0 * random(imagePosition + origin.xy) - 1.0;\n"
         "    float v = random(imagePosition + origin.yx);\n"
         "    float r = sqrt(1.0 - u * u);\n"
         "    float theta = v * 2.0 * M_PI;\n"
         "    vec3 newDirection = vec3(r * cos(theta), r * sin(theta), u);\n"
         "    if (dot(newDirection, normal) < 0.0)\n"
         "        newDirection = -newDirection;\n"
         "\n"
         "    vec4 newRadiance = radiance + vec4(weight.xyz, 1.0) * material.emission;\n"
         "    newRadiance.w = radiance.w;\n"
         "\n"
         "    vec4 newWeight = weight * (material.specular + material.diffuse) * max(0.0, dot(newDirection, normal));\n"
         "    float maxWeight = max(newWeight.x, max(newWeight.y, newWeight.z));\n"
         "    if (maxWeight < 0.01) {\n"
         "        generateRay(imagePosition, newOrigin, newDirection);\n"
         "        newWeight = vec4(1.0);\n"
         "        newRadiance.w = newRadiance.w + 1.0;\n"
         "    }\n"
         "\n"
         "    origin = newOrigin + newDirection * 0.001;\n"
         "    direction = newDirection;\n"
         "    radiance = newRadiance;\n"
         "    weight = newWeight;\n"
         "}\n"
         "\n";
}

void SurfaceShader::setSurfaceShaderUniforms(GLuint program) const
{
    m_random->setRandomNumberGeneratorUniforms(program);
    m_raytracer->setRayGeneratorUniforms(program);
}
