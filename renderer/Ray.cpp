// Copyright (C) 2012 Sami Kyöstilä

#include "Ray.h"
#include "scene/Scene.h"

#include <limits>

Ray::Ray():
    nearest(std::numeric_limits<float>::infinity()),
    material(0)
{
}

bool Ray::hit() const
{
    return nearest != std::numeric_limits<float>::infinity();
}
