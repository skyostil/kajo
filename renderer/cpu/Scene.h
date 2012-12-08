// Copyright (C) 2012 Sami Kyöstilä
#ifndef CPU_SCENE_H
#define CPU_SCENE_H

#include <glm/glm.hpp>
#include <vector>

#include "scene/Scene.h"

namespace cpu
{

using scene::Material;
using scene::Camera;

class Transform
{
public:
    explicit Transform(const glm::mat4 matrix);

    glm::mat4 matrix;
    glm::mat4 invMatrix;
    float determinant;
};

class Sphere
{
public:
    explicit Sphere(const scene::Sphere& sphere);

    Transform transform;
    Material material;
    float radius;
};

class Plane
{
public:
    explicit Plane(const scene::Plane& plane);

    Transform transform;
    Material material;
};

typedef std::vector<Sphere> SphereList;
typedef std::vector<Plane> PlaneList;

class Scene
{
public:
    explicit Scene(const scene::Scene& scene);

    glm::vec4 backgroundColor;

    Camera camera;

    SphereList spheres;
    PlaneList planes;
};

}

#endif
