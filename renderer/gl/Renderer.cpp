// Copyright (C) 2012 Sami Kyöstilä

#include "Renderer.h"
#include "renderer/Image.h"

#include <glm/gtc/matrix_transform.hpp>

namespace gl
{

Renderer::Renderer(const scene::Scene& scene)
{
}

void Renderer::render(Image& image, int xOffset, int yOffset, int width, int height) const
{
#if 0
    const Camera& camera = m_scene->camera;

    const glm::vec4 viewport(0, 0, 1, 1);
    glm::vec3 p1 = glm::unProject(glm::vec3(0.f, 0.f, 0.f), camera.transform, camera.projection, viewport);
    glm::vec3 p2 = glm::unProject(glm::vec3(1.f, 0.f, 0.f), camera.transform, camera.projection, viewport);
    glm::vec3 p3 = glm::unProject(glm::vec3(0.f, 1.f, 0.f), camera.transform, camera.projection, viewport);
    glm::vec3 origin(glm::inverse(camera.transform) * glm::vec4(0.f, 0.f, 0.f, 1.f));

    std::unique_ptr<glm::vec4[]> radianceMap(new glm::vec4[width * height]);

    int samples = 1;
    for (int pass = 1;; pass++)
    {
        if (m_observer && !m_observer(pass, samples, xOffset, yOffset, width, height))
            return;
    }
#endif
}

}
