// Copyright (C) 2012 Sami Kyöstilä
#include "Scene.h"

using namespace scene;

Scene::Scene()
{
};

Material::Material():
    specularExponent(0)
{
};

PointLight::PointLight():
    intensity(1)
{
}
