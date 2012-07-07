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
    float specularExponent;
};

class Sphere
{
public:
    glm::mat4 transform;
    glm::mat4 invTransform;
    Material material;
    float radius;
};

class Plane
{
public:
    glm::mat4 transform;
    glm::mat4 invTransform;
    Material material;
};

class PointLight
{
public:
    PointLight();

    glm::mat4 transform;
    glm::vec4 color;
    float intensity;
};

class Camera
{
public:
    glm::mat4 transform;
    glm::mat4 projection;
};

typedef std::vector<Sphere> SphereList;
typedef std::vector<Plane> PlaneList;
typedef std::vector<PointLight> PointLightList;

class Scene
{
public:
    Scene();

    glm::vec4 backgroundColor;

    Camera camera;

    SphereList spheres;
    PlaneList planes;
    PointLightList pointLights;
};

} // namespace scene

#endif
