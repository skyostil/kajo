// Copyright (C) 2012 Sami Kyöstilä
#ifndef GL_RENDERER_H
#define GL_RENDERER_H

#include "Scene.h"
#include "Raytracer.h"
#include "SurfaceShader.h"
#include "Random.h"
#include "renderer/GLHelpers.h"

#include <memory>

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
    Renderer(const scene::Scene& scene, Image* image);

    void render();

private:
    void drawQuad();

    Image* m_image;

    std::unique_ptr<Scene> m_scene;
    std::unique_ptr<Random> m_random;
    std::unique_ptr<Raytracer> m_raytracer;
    std::unique_ptr<SurfaceShader> m_shader;

    Texture m_originTexture;
    Texture m_directionTexture;
    Texture m_distanceNormalTexture;
    Texture m_tangentTexture;
    Texture m_radianceTexture;
    Texture m_weightTexture;

    Texture m_newOriginTexture;
    Texture m_newDirectionTexture;
    Texture m_newRadianceTexture;
    Texture m_newWeightTexture;

    Buffer m_quadBuffer;

    Program m_tracerProgram;
    Program m_shaderProgram;

    Sampler m_originSampler;
    Sampler m_directionSampler;
    Sampler m_distanceNormalSampler;
    Sampler m_tangentSampler;
    Sampler m_radianceSampler;
    Sampler m_weightSampler;

    Framebuffer m_distanceNormalFramebuffer;
    Framebuffer m_tangentFramebuffer;
    Framebuffer m_nextIterationFramebuffer;
    Framebuffer m_radianceFramebuffer;

    std::unique_ptr<glm::vec4[]> m_radianceMap;
};

}

#endif
