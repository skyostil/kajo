// Copyright (C) 2012 Sami Kyöstilä

#include "Raytracer.h"
#include "Scene.h"
#include "renderer/GLHelpers.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace gl;

Raytracer::Raytracer(Scene* scene):
    m_scene(scene)
{
}

void Raytracer::writeRayGenerator(std::ostringstream& s) const
{
    s << "uniform vec3 imageOrigin;\n"
         "uniform vec3 imageRight;\n"
         "uniform vec3 imageDown;\n"
         "uniform vec3 rayOrigin;\n"
         "uniform vec2 invPixelSize;\n"
         "\n"
         "void generateRay(vec2 imagePosition, out vec3 origin, out vec3 direction)\n"
         "{\n"
         "    vec2 jitter = vec2(random(imagePosition + direction.zy),\n"
         "                       random(imagePosition + direction.xz)) * invPixelSize;\n"
         "    origin = rayOrigin;\n"
         "    direction = normalize(imageOrigin +\n"
         "                          imageRight * (imagePosition.x + jitter.x) +\n"
         "                          imageDown * (imagePosition.y + jitter.y) - rayOrigin);\n"
         "}\n"
         "\n";
}

void Raytracer::setRayGeneratorUniforms(GLuint program) const
{
    const Camera& camera = m_scene->camera;
    const glm::vec4 viewport(0, 0, 1, 1);
    glm::vec3 p1 = glm::unProject(glm::vec3(0.f, 0.f, 0.f), camera.transform, camera.projection, viewport);
    glm::vec3 p2 = glm::unProject(glm::vec3(1.f, 0.f, 0.f), camera.transform, camera.projection, viewport) - p1;
    glm::vec3 p3 = glm::unProject(glm::vec3(0.f, 1.f, 0.f), camera.transform, camera.projection, viewport) - p1;
    glm::vec3 origin(glm::inverse(camera.transform) * glm::vec4(0.f, 0.f, 0.f, 1.f));

    GLint rendererViewport[4];
    glGetIntegerv(GL_VIEWPORT, &rendererViewport[0]);

    glUniform3f(uniform(program, "imageOrigin"), p1.x, p1.y, p1.z);
    glUniform3f(uniform(program, "imageRight"), p2.x, p2.y, p2.z);
    glUniform3f(uniform(program, "imageDown"), p3.x, p3.y, p3.z);
    glUniform3f(uniform(program, "rayOrigin"), origin.x, origin.y, origin.z);
    glUniform2f(uniform(program, "invPixelSize"), 1.f / rendererViewport[2], 1.f / rendererViewport[3]);
    ASSERT_GL();
}

void Raytracer::writeRayIntersector(std::ostringstream& s) const
{
    s << "struct SurfacePoint {\n"
         "    float objectIndex;\n"
         "    float minDistance;\n"
         "    float maxDistance;\n"
         "    vec3 position;\n"
         "    vec3 view;\n"
         "    vec3 normal;\n"
         "    vec3 tangent;\n"
         "    vec3 binormal;\n"
         "};\n"
         "\n";

    size_t objectIndex = 0;
    for (size_t i = 0; i < m_scene->planes.size(); i++) {
        std::string name = "intersectPlane" + std::to_string(i);
        m_scene->planes[i].writeIntersector(s, name, objectIndex++);
        s << "\n";
    }

    for (size_t i = 0; i < m_scene->spheres.size(); i++) {
        std::string name = "intersectSphere" + std::to_string(i);
        m_scene->spheres[i].writeIntersector(s, name, objectIndex++);
        s << "\n";
    }

    s << "SurfacePoint traceRay(vec3 origin, vec3 direction)\n"
         "{\n"
         "    SurfacePoint result;\n"
         "    result.minDistance = 0.0;\n"
         "    result.objectIndex = -1.0;\n"
         "    result.maxDistance = 1e16;\n"
         "\n";

    for (size_t i = 0; i < m_scene->planes.size(); i++) {
        std::string name = "intersectPlane" + std::to_string(i);
        s << "    " << name << "(origin, direction, result.minDistance, "
                               "result.maxDistance, result.normal, "
                               "result.objectIndex);\n";
    }

    for (size_t i = 0; i < m_scene->spheres.size(); i++) {
        std::string name = "intersectSphere" + std::to_string(i);
        s << "    " << name << "(origin, direction, result.minDistance, "
                               "result.maxDistance, result.normal, "
                               "result.objectIndex);\n";
    }

    s << "    result.view = direction;\n"
         "    result.position = direction * result.maxDistance;\n"
         "    return result;\n"
         "}\n"
         "\n";

    s << "bool rayCanReach(vec3 origin, vec3 direction, int objectIndex)\n"
         "{\n"
         "    SurfacePoint result = traceRay(origin, direction);\n"
         "    return int(result.objectIndex) == objectIndex;\n"
         "}\n"
         "\n";
}
