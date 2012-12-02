// Copyright (C) 2012 Sami Kyöstilä
#ifndef GL_RENDERER_H
#define GL_RENDERER_H

class Image;

namespace scene
{
class Scene;
}

namespace gl
{

class Renderer
{
public:
    Renderer(const scene::Scene& scene);

    void render(Image& image, int xOffset, int yOffset, int width, int height) const;
};

}

#endif