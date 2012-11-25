// Copyright (C) 2012 Sami Kyöstilä

#include "Random.h"
#include "Ray.h"
#include "Raytracer.h"
#include "Renderer.h"
#include "Shader.h"
#include "SurfacePoint.h"
#include "renderer/Image.h"
#include "scene/Scene.h"

#include <glm/gtc/matrix_transform.hpp>

using namespace cpu;

Renderer::Renderer(Scene* scene):
    m_scene(scene),
    m_raytracer(new Raytracer(scene)),
    m_shader(new Shader(scene, m_raytracer.get())),
    m_samples(32)
{
}

void Renderer::render(Image& image, int xOffset, int yOffset, int width, int height) const
{
    Random random(0715517 * (yOffset + 1));
    const Camera& camera = m_scene->camera;

    const glm::vec4 viewport(0, 0, 1, 1);
    glm::vec3 p1 = glm::unProject(glm::vec3(0.f, 0.f, 0.f), camera.transform, camera.projection, viewport);
    glm::vec3 p2 = glm::unProject(glm::vec3(1.f, 0.f, 0.f), camera.transform, camera.projection, viewport);
    glm::vec3 p3 = glm::unProject(glm::vec3(0.f, 1.f, 0.f), camera.transform, camera.projection, viewport);
    glm::vec3 origin(glm::inverse(camera.transform) * glm::vec4(0.f, 0.f, 0.f, 1.f));

    std::unique_ptr<glm::vec4[]> radianceMap(new glm::vec4[width * height]);

    int samplesPerAxis = sqrt(m_samples);
    float pixelWidth = 1.f / image.width;
    float pixelHeight = 1.f / image.height;
    float sampleWidth = pixelWidth / samplesPerAxis;
    float sampleHeight = pixelHeight / samplesPerAxis;

    for (int pass = 1;; pass++)
    {
        for (int y = yOffset; y < yOffset + height; y++)
        {
            for (int x = xOffset; x < xOffset + width; x++)
            {
                glm::vec4 radiance;
                for (int sampleY = 0; sampleY < samplesPerAxis; sampleY++)
                {
                    for (int sampleX = 0; sampleX < samplesPerAxis; sampleX++)
                    {
                        glm::vec4 offset = random.generate() * .5f + glm::vec4(.5f);
                        float sx = x * pixelWidth + sampleX * sampleWidth + offset.x * sampleWidth;
                        float sy = (image.height - y) * pixelHeight + sampleY * sampleHeight + offset.y * sampleHeight;
                        glm::vec3 direction = p1 + (p2 - p1) * sx + (p3 - p1) * sy - origin;
                        direction = glm::normalize(direction);

                        Ray ray;
                        ray.origin = origin;
                        ray.direction = direction;

                        SurfacePoint surfacePoint = m_raytracer->trace(ray);
                        radiance += m_shader->shade(surfacePoint, random);
                    }
                }
                // Combine new sample with the previous passes.
                glm::vec4& totalRadiance = radianceMap[(y - yOffset) * width + (x - xOffset)];
                totalRadiance += radiance / m_samples;

                glm::vec4 pixel = Image::linearToSRGB(glm::clamp(totalRadiance / pass, glm::vec4(0), glm::vec4(1)));
                pixel.a = 1;
                image.pixels[y * image.width + x] = Image::colorToRGBA8(pixel);
            }
            if (m_observer && !m_observer(pass, m_samples, 0, y, width, 1))
                return;
        }
    }
}

void Renderer::setObserver(RenderObserver observer)
{
    m_observer = observer;
}
