// Copyright (C) 2012 Sami Kyöstilä
#include "Scene.h"
#include "scene/Scene.h"
#include "ShaderUtil.h"

#include <algorithm>

using namespace gl;

Material::Material(const scene::Material& material):
    material(material)
{
}

void Material::writeInitializer(std::ostringstream& s) const
{
    s << "Material(vec4(";
    writeVec4(s, material.ambient);
    s << "), vec4(";
    writeVec4(s, 1 * material.diffuse + 0 * material.specular);
    s << "), vec4(";
    writeVec4(s, 1 * material.specular + 0 * material.diffuse);
    s << "), vec4(";
    writeVec4(s, material.emission);
    s << "), vec4(";
    writeVec4(s, material.transparency);
    s << "), ";
    writeFloat(s, material.specularExponent);
    s << ", ";
    writeFloat(s, material.refractiveIndex);
    s << ", ";

    float totalDiffuse = material.diffuse.x + material.diffuse.y + material.diffuse.z;
    float totalSpecular = material.specular.x + material.specular.y + material.specular.z;
    float totalTransparency = material.transparency.x + material.transparency.y + material.transparency.z;

    if (totalDiffuse + totalSpecular + totalTransparency > 0) {
        float transparencyProbability = totalTransparency / (totalDiffuse + totalSpecular + totalTransparency);
        writeFloat(s, transparencyProbability);
    } else {
        writeFloat(s, 0);
    }
    s << ", ";

    if (totalDiffuse + totalSpecular > 0) {
        float diffuseProbability = totalDiffuse / (totalDiffuse + totalSpecular);
        writeFloat(s, diffuseProbability);
    } else {
        writeFloat(s, 0);
    }
    s << ")";
}

Transform::Transform(const glm::mat4 matrix):
    matrix(matrix),
    invMatrix(glm::inverse(matrix)),
    determinant(glm::determinant(matrix))
{
}

void Transform::writeMatrixInitializer(std::ostringstream& s) const
{
    s << "mat4(";
    writeMatrix(s, matrix);
    s << ")";
}

void Transform::writeInverseMatrixInitializer(std::ostringstream& s) const
{
    s << "mat4(";
    writeMatrix(s, invMatrix);
    s << ")";
}

Sphere::Sphere(const scene::Sphere& sphere):
    transform(sphere.transform),
    material(sphere.material),
    radius(sphere.radius)
{
}

void Sphere::writeIntersector(std::ostringstream& s, const std::string& name, size_t objectIndex) const
{
    s << "void " << name << "(vec3 origin, vec3 direction, inout float minDistance,\n"
         "        inout float maxDistance, inout vec3 normal, inout vec3 tangent,\n"
         "        inout float objectIndex)\n";
    s << "{\n";
    s << "    mat4 transform = ";
    transform.writeMatrixInitializer(s);
    s << ";\n";

    s << "    mat4 invTransform = ";
    transform.writeInverseMatrixInitializer(s);
    s << ";\n";

    s << "    float determinant = ";
    writeFloat(s, transform.determinant);
    s << ";\n";

    s << "    float radius2 = ";
    writeFloat(s, radius * radius);
    s << ";\n";

    s << "    float localObjectIndex = ";
    writeFloat(s, objectIndex);
    s << ";\n";

    s << "    vec3 localDir = mat3(invTransform) * direction;\n"
         "    vec3 localOrigin = (invTransform * vec4(origin, 1.0)).xyz;\n"
         "    float a = dot(localDir, localDir);\n"
         "    float b = 2.0 * dot(localDir, localOrigin);\n"
         "    float c = dot(localOrigin, localOrigin) - radius2;\n"
         "    float discr = b * b - 4.0 * a * c;\n"
         "\n"
         "    if (discr < 0)\n"
         "        return;\n"
         "\n"
         "    float q;\n"
         "    if (b < 0)\n"
         "        q = (-b - sqrt(discr)) * .5;\n"
         "    else\n"
         "        q = (-b + sqrt(discr)) * .5;\n"
         "\n"
         "    float t0 = q / a;\n"
         "    float t1 = c / q;\n"
         "\n"
         "    if (t0 > t1) {\n"
         "        float tmp = t1;\n"
         "        t1 = t0;\n"
         "        t0 = tmp;\n"
         "    }\n"
         "\n"
         "    if (t1 > 0) {\n"
         "        if (t0 < 0)\n"
         "            t0 = t1;\n"
         "\n"
         "        t0 = determinant * t0;\n"
         "        if (t0 > minDistance && t0 < maxDistance) {\n"
         "            mat3 rotation = mat3(transform);\n"
         "            maxDistance = t0;\n"
         "            normal = localOrigin + localDir * t0;\n"
         "            normal = normalize(rotation * normal);\n"
         "            tangent = cross(normal, vec3(0.0, 1.0, 0.0));\n"
         "            objectIndex = localObjectIndex;\n"
         "        }\n"
         "    }\n"
         "}\n";
}

Plane::Plane(const scene::Plane& plane):
    transform(plane.transform),
    material(plane.material)
{
}

void Plane::writeIntersector(std::ostringstream& s, const std::string& name, size_t objectIndex) const
{
    s << "void " << name << "(vec3 origin, vec3 direction, inout float minDistance,\n"
         "        inout float maxDistance, inout vec3 normal, inout vec3 tangent,\n"
         "        inout float objectIndex)\n";
    s << "{\n";
    s << "    mat4 transform = ";
    transform.writeMatrixInitializer(s);
    s << ";\n";

    s << "    mat4 invTransform = ";
    transform.writeInverseMatrixInitializer(s);
    s << ";\n";

    s << "    float determinant = ";
    writeFloat(s, transform.determinant);
    s << ";\n";

    s << "    float localObjectIndex = ";
    writeFloat(s, objectIndex);
    s << ";\n";

    s << "    vec3 localDir = mat3(invTransform) * direction;\n"
         "    vec3 localOrigin = (invTransform * vec4(origin, 1.0)).xyz;\n"
         "    vec3 localNormal = vec3(0.0, 1.0, 0.0);\n"
         "    vec3 localTangent = vec3(1.0, 0.0, 0.0);\n"
         "    mat3 rotation = mat3(transform);\n"
         "    float denom = dot(localDir, localNormal);\n"
         "    float t = -dot(localOrigin, localNormal) / denom;\n"
         "    if (t > minDistance && t < maxDistance) {\n"
         "        maxDistance = t;\n"
         "        normal = rotation * -localNormal;\n"
         "        tangent = rotation * localTangent;\n"
         "        objectIndex = localObjectIndex;\n"
         "    }\n"
         "}\n";
}

Scene::Scene(const scene::Scene& scene):
    backgroundColor(scene.backgroundColor),
    camera(scene.camera)
{
    for (const scene::Sphere& sphere: scene.spheres)
        spheres.push_back(Sphere(sphere));
    for (const scene::Plane& plane: scene.planes)
        planes.push_back(Plane(plane));
}

size_t Scene::objectIndex(const Plane& plane) const
{
    return &plane - &planes[0];
}

size_t Scene::objectIndex(const Sphere& sphere) const
{
    return planes.size() + &sphere - &spheres[0];
}
