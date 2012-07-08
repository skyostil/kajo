// Copyright (C) 2012 Sami Kyöstilä
#include "PrecalculatedScene.h"
#include "scene/Scene.h"

#include <algorithm>

PrecalculatedScene::PrecalculatedScene(scene::Scene* scene):
    m_scene(scene)
{
    prepare();
}

void PrecalculatedScene::prepare()
{
    prepareAll(m_scene->spheres, sphereTransforms);
    prepareAll(m_scene->planes, planeTransforms);
    prepareAll(m_scene->pointLights, pointLightTransforms);
}

template <typename ObjectType>
void PrecalculatedScene::prepareAll(std::vector<ObjectType>& objects, TransformDataList& transformDataList)
{
    std::for_each(objects.begin(), objects.end(), [&](ObjectType& object){
        TransformData transformData;
        transformData.invTransform = glm::inverse(object.transform);
        transformData.determinant = glm::determinant(object.transform);
        transformDataList.push_back(transformData);
    });
}
