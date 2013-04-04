// Copyright (C) 2012 Sami Kyöstilä
#ifndef GL_SCENE_H
#define GL_SCENE_H

#include <glm/glm.hpp>
#include <vector>
#include <sstream>

#include "scene/Scene.h"

namespace gl
{

using scene::Camera;

class Material
{
public:
    explicit Material(const scene::Material& material);

    void writeInitializer(std::ostringstream& s) const;

    scene::Material material;
};

class Transform
{
public:
    explicit Transform(const glm::mat4 matrix);

    void writeMatrixInitializer(std::ostringstream& s) const;
    void writeInverseMatrixInitializer(std::ostringstream& s) const;

    glm::mat4 matrix;
    glm::mat4 invMatrix;
    float determinant;
};

class Sphere
{
public:
    explicit Sphere(const scene::Sphere& sphere);

    void writeIntersector(std::ostringstream& s, const std::string& name, size_t objectIndex) const;

    Transform transform;
    Material material;
    float radius;
};

class Plane
{
public:
    explicit Plane(const scene::Plane& plane);

    void writeIntersector(std::ostringstream& s, const std::string& name, size_t objectIndex) const;

    Transform transform;
    Material material;
};

typedef std::vector<Sphere> SphereList;
typedef std::vector<Plane> PlaneList;

class Scene
{
public:
    explicit Scene(const scene::Scene& scene);

    size_t objectIndex(const Plane& plane) const;
    size_t objectIndex(const Sphere& sphere) const;

    glm::vec4 backgroundColor;

    Camera camera;

    SphereList spheres;
    PlaneList planes;
};

}

#endif
