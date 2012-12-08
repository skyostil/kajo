// Copyright (C) 2012 Sami Kyöstilä
#ifndef GL_SCENE_H
#define GL_SCENE_H

#include <glm/glm.hpp>
#include <vector>
#include <sstream>

#include "scene/Scene.h"

namespace gl
{

using scene::Material;
using scene::Camera;

class Transform
{
public:
    Transform(const glm::mat4 matrix);

    void writeMatrixInitializer(std::ostringstream& s) const;
    void writeInverseMatrixInitializer(std::ostringstream& s) const;

    glm::mat4 matrix;
    glm::mat4 invMatrix;
    float determinant;
};

class Sphere
{
public:
    Sphere(const scene::Sphere& sphere);

    void writeIntersector(std::ostringstream& s, const std::string& name) const;

    Transform transform;
    Material material;
    float radius;
};

class Plane
{
public:
    Plane(const scene::Plane& plane);

    void writeIntersector(std::ostringstream& s, const std::string& name) const;

    Transform transform;
    Material material;
};

typedef std::vector<Sphere> SphereList;
typedef std::vector<Plane> PlaneList;

class Scene
{
public:
    Scene(const scene::Scene& scene);

    glm::vec4 backgroundColor;

    Camera camera;

    SphereList spheres;
    PlaneList planes;
};

}

#endif

