// Copyright (C) 2012 Sami Kyöstilä
#ifndef PRECALCULATED_SCENE_H
#define PRECALCULATED_SCENE_H

#include <glm/glm.hpp>
#include <vector>

#include "scene/Scene.h"

namespace cpu
{

using scene::Scene;
using scene::Plane;
using scene::Material;
using scene::Sphere;
using scene::Camera;

class TransformData
{
public:
    glm::mat4 invTransform;
    float determinant;
};

typedef std::vector<TransformData> TransformDataList;

class PrecalculatedScene
{
public:
    PrecalculatedScene(Scene* scene);

    TransformDataList sphereTransforms;
    TransformDataList planeTransforms;

private:
    void prepare();

    template <typename ObjectType>
    void prepareAll(std::vector<ObjectType>& objects, TransformDataList& transformDataList);

    Scene* m_scene;
};

}

#endif
