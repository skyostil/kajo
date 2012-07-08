// Copyright (C) 2012 Sami Kyöstilä
#include "Renderer.h"
#include "Surface.h"
#include "scene/Scene.h"

#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <lodepng.h>

void buildTestScene(scene::Scene& scene)
{
    using namespace scene;

    glm::vec4 colors[] =
    {
        glm::vec4(0, .3f, 0, 1),
        glm::vec4(.3f, 0, 0, 1),
        glm::vec4(0, 0, .3f, 1),
        glm::vec4(.3f, .3f, .3f, 1),
    };

    for (int i = 0; i < 4; i++)
    {
        Sphere sphere;
        sphere.radius = 1.f;
        sphere.material.ambient = colors[i] * 0.1f;
        sphere.material.diffuse = colors[i];
        sphere.material.specularExponent = 20;
        sphere.material.reflectivity = 0.5f;
        sphere.transform = glm::translate(sphere.transform, glm::vec3(i * 2 - 2, 0, 0));
        scene.spheres.push_back(sphere);
    }

    Plane ground;
    ground.transform = glm::translate(ground.transform, glm::vec3(0, 1, 0));
    ground.material.diffuse = glm::vec4(.4f, .4f, .4f, 1);
    ground.material.ambient = ground.material.diffuse * 0.05f;
    ground.material.reflectivity = 0.5f;
    scene.planes.push_back(ground);

    PointLight light;
    light.transform = glm::translate(glm::mat4(1), glm::vec3(4, 4, 0));
    light.color = glm::vec4(1, 1, 1, 1);
    light.intensity = 30;
    scene.pointLights.push_back(light);

    Camera camera;
    camera.projection = glm::perspective(45.f, 4.f / 3.f, .1f, 100.f);
    camera.transform = glm::lookAt(glm::vec3(-4, -1.2f, 5), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    scene.camera = camera;

    scene.backgroundColor = glm::vec4(.2f, .2f, .2f, 1);
}

void render(Surface& surface, scene::Scene& scene)
{
    Renderer renderer(&scene);
    renderer.render(surface, 0, 0, surface.width, surface.height);
}

void save(Surface& surface, const std::string& fileName)
{
    unsigned error =
        lodepng::encode(fileName,
                        reinterpret_cast<const unsigned char*>(&surface.pixels[0]),
                        surface.width, surface.height);
    if (error)
    {
        std::cerr << "PNG encode failure: " << lodepng_error_text(error) << std::endl;
    }
}

int main(int argc, char** argv)
{
    scene::Scene scene;

    buildTestScene(scene);

    Surface surface(640, 480);
    render(surface, scene);
    save(surface, "out.png");
}
