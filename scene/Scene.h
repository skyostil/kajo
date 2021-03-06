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
    Material();

    glm::vec4 ambient;
    glm::vec4 diffuse;
    glm::vec4 specular;
    glm::vec4 emission;
    glm::vec4 transparency;
    float specularExponent;
    float refractiveIndex;
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
};

class Camera
{
public:
    glm::mat4 transform;
    glm::mat4 projection;
};

typedef std::vector<Sphere> SphereList;
typedef std::vector<Plane> PlaneList;

class Scene
{
public:
    Scene();

    glm::vec4 backgroundColor;

    Camera camera;

    SphereList spheres;
    PlaneList planes;
};

} // namespace scene

#endif
