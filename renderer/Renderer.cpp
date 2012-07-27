// Copyright (C) 2012 Sami Kyöstilä

#include "Renderer.h"
#include "Random.h"
#include "Ray.h"
#include "Raytracer.h"
#include "Shader.h"
#include "Surface.h"
#include "scene/Scene.h"

#include <glm/gtc/matrix_transform.hpp>

Renderer::Renderer(scene::Scene* scene):
    m_scene(scene),
    m_raytracer(new Raytracer(scene)),
    m_shader(new Shader(scene, m_raytracer.get())),
    m_samples(16)
{
}

void Renderer::render(Surface& surface, int xOffset, int yOffset, int width, int height) const
{
    Random random(0715517 * (yOffset + 1));
    const scene::Camera& camera = m_scene->camera;

    const glm::vec4 viewport(0, 0, 1, 1);
    glm::vec3 p1 = glm::unProject(glm::vec3(0.f, 0.f, 0.f), camera.transform, camera.projection, viewport);
    glm::vec3 p2 = glm::unProject(glm::vec3(1.f, 0.f, 0.f), camera.transform, camera.projection, viewport);
    glm::vec3 p3 = glm::unProject(glm::vec3(0.f, 1.f, 0.f), camera.transform, camera.projection, viewport);
    glm::vec3 origin(glm::inverse(camera.transform) * glm::vec4(0.f, 0.f, 0.f, 1.f));

    float pixelWidth = 1.f / surface.width;
    float pixelHeight = 1.f / surface.height;
    for (int y = yOffset; y < yOffset + height; y++)
    {
        for (int x = xOffset; x < xOffset + width; x++)
        {
            glm::vec4 radiance;
            for (unsigned s = 0; s < m_samples; s++)
            {
                glm::vec4 offset = random.generate();
                float sx = x * pixelWidth + pixelWidth * offset.x;
                float sy = (surface.height - y) * pixelHeight + pixelHeight * offset.y;
                glm::vec3 direction = p1 + (p2 - p1) * sx + (p3 - p1) * sy - origin;
                direction = glm::normalize(direction);

                Ray ray;
                ray.origin = origin;
                ray.direction = direction;

                m_raytracer->trace(ray);
                radiance += m_shader->shade(ray, random) * (1.f / m_samples);
            }
            glm::vec4 pixel = Surface::linearToSRGB(glm::clamp(radiance, glm::vec4(0), glm::vec4(1)));
            pixel.a = 1;
            surface.pixels[y * surface.width + x] = Surface::colorToRGBA8(pixel);
        }
    }
}
