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

uint32_t colorToRGBA8(const glm::vec4& color)
{
    int r = std::max(0, std::min(0xff, static_cast<int>(color.r * 255.f)));
    int g = std::max(0, std::min(0xff, static_cast<int>(color.g * 255.f)));
    int b = std::max(0, std::min(0xff, static_cast<int>(color.b * 255.f)));
    int a = std::max(0, std::min(0xff, static_cast<int>(color.a * 255.f)));
    uint32_t pixel = (a << 24) | (r << 16) | (g << 8) | b;
    return pixel;
}

void dump(const glm::vec4& v)
{
    printf("(%f %f %f %f)\n", v.x, v.y, v.z, v.w);
}

void Renderer::render(Surface& surface, int xOffset, int yOffset, int width, int height)
{
    for (int y = yOffset; y < yOffset + height; y++)
    {
        for (int x = xOffset; x < xOffset + width; x++)
        {
            glm::vec4 color = m_scene->backgroundColor;
            surface.pixels[y * surface.width + x] = colorToRGBA8(color);
        }
    }
}
