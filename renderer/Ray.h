// Copyright (C) 2012 Sami Kyöstilä
#ifndef RAY_H
#define RAY_H

#include <glm/glm.hpp>
#include <cstdint>

namespace scene
{
    class Material;
}

class Random;

class Ray
{
public:
    Ray(Random& random);

    Random& random;
    glm::vec3 origin;
    glm::vec3 direction;

    bool hit() const;

    // Result
    intptr_t objectId;
    float minDistance; // Assumed to be >= 0
    float maxDistance;
    glm::vec3 normal;
    glm::vec3 tangent;
    glm::vec3 binormal;
    glm::vec3 hitPos;
    const scene::Material* material;
};

#endif
