// Copyright (C) 2012 Sami Kyöstilä
#include "Scene.h"

using namespace scene;

Scene::Scene()
{
};

Material::Material():
    specularExponent(0),
    reflectivity(0),
    transparency(0),
    refractiveIndex(1),
    checkerboard(false)
{
};

PointLight::PointLight():
    intensity(1)
{
}
