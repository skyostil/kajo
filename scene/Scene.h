// Copyright (C) 2012 Sami Kyöstilä
#ifndef SCENE_H
#define SCENE_H

#include <glm/glm.hpp>
#include <vector>

namespace scene
{

class Material
{
public:
    glm::vec3 color;
};

class Sphere
{
public:
    glm::mat4 transform;
    Material material;
    float radius;
};

class Plane
{
public:
    glm::mat4 transform;
    Material material;
    float distance;
};

class Camera
{
public:
    glm::mat4 projection;
    glm::mat4 view;
};

typedef std::vector<Sphere> SphereList;
typedef std::vector<Plane> PlaneList;

class Scene
{
public:
    Scene();

    Camera camera;

    SphereList spheres;
    PlaneList planes;
};

} // namespace scene

#endif
