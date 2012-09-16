// Copyright (C) 2012 Sami Kyöstilä

#include "Ray.h"
#include "scene/Scene.h"

#include <limits>

Ray::Ray(Random& random):
    random(random),
    objectId(0),
    minDistance(0),
    maxDistance(std::numeric_limits<float>::infinity()),
    material(0)
{
}

bool Ray::hit() const
{
    return objectId;
}
