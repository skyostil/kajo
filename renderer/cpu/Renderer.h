// Copyright (C) 2012 Sami Kyöstilä
#ifndef RENDERER_H
#define RENDERER_H

#include <memory>

#include "Raytracer.h"
#include "Shader.h"

class Image;

namespace cpu
{

class Scene;

typedef std::function<bool(int pass, int samples, int xOffset, int yOffset, int width, int height)> RenderObserver;

class Renderer
{
public:
    Renderer(Scene* scene);

    void setObserver(RenderObserver observer);
    void render(Image& image, int xOffset, int yOffset, int width, int height) const;

private:
    Scene* m_scene;
    std::unique_ptr<Raytracer> m_raytracer;
    std::unique_ptr<Shader> m_shader;
    unsigned m_samples;
    RenderObserver m_observer;
};

}

#endif
