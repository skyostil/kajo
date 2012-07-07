// Copyright (C) 2012 Sami Kyöstilä

#include "Surface.h"

Surface::Surface(int width, int height):
    width(width),
    height(height),
    pixels(new uint32_t[width * height])
{
}

uint32_t Surface::colorToRGBA8(const glm::vec4& color)
{
    int r = std::max(0, std::min(0xff, static_cast<int>(color.r * 255.f)));
    int g = std::max(0, std::min(0xff, static_cast<int>(color.g * 255.f)));
    int b = std::max(0, std::min(0xff, static_cast<int>(color.b * 255.f)));
    int a = std::max(0, std::min(0xff, static_cast<int>(color.a * 255.f)));
    uint32_t pixel = (a << 24) | (r << 16) | (g << 8) | b;
    return pixel;
}
