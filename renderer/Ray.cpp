// Copyright (C) 2012 Sami Kyöstilä

#include "Ray.h"
#include "scene/Scene.h"

#include <limits>

Ray::Ray():
    hit(false),
    minDistance(0),
    maxDistance(std::numeric_limits<float>::infinity()),
    material(0)
{
}
