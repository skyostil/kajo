// Copyright (C) 2012 Sami Kyöstilä
#ifndef PRECALCULATED_SCENE_H
#define PRECALCULATED_SCENE_H

#include <glm/glm.hpp>
#include <vector>

namespace scene
{
    class Scene;
}

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
    PrecalculatedScene(scene::Scene* scene);

    TransformDataList sphereTransforms;
    TransformDataList planeTransforms;

private:
    void prepare();

    template <typename ObjectType>
    void prepareAll(std::vector<ObjectType>& objects, TransformDataList& transformDataList);

    scene::Scene* m_scene;
};

#endif
