// Copyright (C) 2012 Sami Kyöstilä

#include "SurfacePoint.h"

using namespace cpu;

SurfacePoint::SurfacePoint():
    objectId(0),
    material(0)
{
}

bool SurfacePoint::valid() const
{
    return objectId;
}
