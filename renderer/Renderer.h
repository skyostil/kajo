// Copyright (C) 2012 Sami Kyöstilä
#ifndef RENDERER_H
#define RENDERER_H

#include "scene/Scene.h"

#include <memory>
#include <stdint.h>

class Surface
{
public:
    Surface(int width, int height);

    int width;
    int height;
    std::unique_ptr<uint32_t[]> pixels;
};

class Renderer
{
public:
    Renderer(scene::Scene* scene);

    void render(Surface& surface, int xOffset, int yOffset, int width, int height);

private:
    scene::Scene* m_scene;
};

#endif
