// Copyright (C) 2012 Sami Kyöstilä
#ifndef SURFACEPOINT_H
#define SURFACEPOINT_H

#include <glm/glm.hpp>
#include <cstdint>

namespace scene
{
    class Material;
}

class SurfacePoint
{
public:
    SurfacePoint();

    bool valid() const;

    intptr_t objectId;
    float minDistance; // Assumed to be >= 0
    float maxDistance;
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 tangent;
    glm::vec3 binormal;
    const scene::Material* material;
};

#endif
