// Copyright (C) 2012 Sami Kyöstilä

#include "Renderer.h"

Surface::Surface(int width, int height):
    width(width),
    height(height),
    pixels(new uint32_t[width * height])
{
}

Renderer::Renderer(scene::Scene* scene):
    m_scene(scene)
{
}

void Renderer::render(Surface& surface, int x, int y, int width, int height)
{
}
