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
    glm::vec4 color;
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

class OmniLight
{
public:
    glm::mat4 transform;
    glm::mat4 color;
};

class Camera
{
public:
    glm::mat4 transform;
    glm::mat4 projection;
};

typedef std::vector<Sphere> SphereList;
typedef std::vector<Plane> PlaneList;
typedef std::vector<OmniLight> OmniLightList;

class Scene
{
public:
    Scene();

    glm::vec4 backgroundColor;

    Camera camera;

    SphereList spheres;
    PlaneList planes;
    OmniLightList lights;
};

} // namespace scene

#endif
