// Copyright (C) 2012 Sami Kyöstilä
#ifndef SURFACE_H
#define SURFACE_H

#include "scene/Scene.h"

#include <memory>
#include <stdint.h>

class Surface
{
public:
    Surface(int width, int height);

    static uint32_t colorToRGBA8(const glm::vec4& color);

    int width;
    int height;
    std::unique_ptr<uint32_t[]> pixels;
};

#endif
