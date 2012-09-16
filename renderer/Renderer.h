// Copyright (C) 2012 Sami Kyöstilä
#ifndef RENDERER_H
#define RENDERER_H

#include <memory>

class Raytracer;
class Shader;
class Surface;

namespace scene
{
    class Scene;
}

typedef std::function<bool(int pass, int samples, int xOffset, int yOffset, int width, int height)> RenderObserver;

class Renderer
{
public:
    Renderer(scene::Scene* scene);

    void setObserver(RenderObserver observer);
    void render(Surface& surface, int xOffset, int yOffset, int width, int height) const;

private:
    scene::Scene* m_scene;
    std::unique_ptr<Raytracer> m_raytracer;
    std::unique_ptr<Shader> m_shader;
    unsigned m_samples;
    RenderObserver m_observer;
};

#endif
