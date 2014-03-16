// Copyright (C) 2012 Sami Kyöstilä
#include "Util.h"

void dump(const glm::vec3& v)
{
    printf("(%f %f %f)\n", v.x, v.y, v.z);
}

void dump(const glm::vec4& v)
{
    printf("(%f %f %f %f)\n", v.x, v.y, v.z, v.w);
}
