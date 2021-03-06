// Copyright (C) 2012 Sami Kyöstilä
#ifndef CPU_RAY_H
#define CPU_RAY_H

#include <glm/glm.hpp>
#include <cstdint>

namespace cpu
{

class Ray
{
public:
    Ray();

    glm::vec3 origin;
    glm::vec3 direction;
    float minDistance; // Assumed to be >= 0
    float maxDistance;
};

}

#endif
