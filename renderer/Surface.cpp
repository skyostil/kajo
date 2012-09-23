// Copyright (C) 2012 Sami Kyöstilä

#include "Surface.h"
#include <iostream>
#include <lodepng.h>

Surface::Surface(int width, int height):
    width(width),
    height(height),
    pixels(new uint32_t[width * height])
{
}

glm::vec4 Surface::linearToSRGB(const glm::vec4& color)
{
    return glm::pow(color, glm::vec4(1 / 2.2f));
}

uint32_t Surface::colorToRGBA8(const glm::vec4& color)
{
    int r = static_cast<int>(color.r * 255.f + .5f);
    int g = static_cast<int>(color.g * 255.f + .5f);
    int b = static_cast<int>(color.b * 255.f + .5f);
    int a = static_cast<int>(color.a * 255.f + .5f);
    uint32_t pixel = (a << 24) | (r << 16) | (g << 8) | b;
    return pixel;
}

bool Surface::save(const std::string& fileName) const
{
    std::unique_ptr<uint32_t[]> bgraPixels(new uint32_t[width * height]);

    // Fix the channel ordering for PNG
    for (int i = 0; i < width * height; i++)
        bgraPixels[i] =
            (pixels[i] & 0xff00ff00) |
            ((pixels[i] & 0x00ff0000) >> 16) |
            ((pixels[i] & 0x000000ff) << 16);

    unsigned error =
        lodepng::encode(fileName,
                        reinterpret_cast<const unsigned char*>(&bgraPixels[0]),
                        width, height);

    if (error)
        std::cerr << "PNG encode failure: " << lodepng_error_text(error) << std::endl;
    return !error;
}
