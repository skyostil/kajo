// Copyright (C) 2012 Sami Kyöstilä
#ifndef CPU_RENDERER_H
#define CPU_RENDERER_H

#include "renderer/Renderer.h"
#include "Raytracer.h"
#include "Shader.h"

class Image;

namespace scene
{
class Scene;
}

namespace cpu
{

class Renderer: public ::Renderer
{
public:
    Renderer(const scene::Scene& scene);

    virtual void setObserver(RenderObserver observer) override;
    virtual void render(Image& image, int xOffset, int yOffset, int width, int height) const override;

private:
    std::unique_ptr<Scene> m_scene;
    std::unique_ptr<Raytracer> m_raytracer;
    std::unique_ptr<Shader> m_shader;
    unsigned m_samples;
    RenderObserver m_observer;
};

}

#endif
