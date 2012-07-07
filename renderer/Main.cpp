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

    Sphere sphere;
    sphere.radius = 1.f;
    sphere.material.color = glm::vec4(0, .3f, 0, 1);
    sphere.transform = glm::translate(sphere.transform, glm::vec3(0, 0, 0));
    scene.spheres.push_back(sphere);

    Plane ground;
    ground.transform = glm::translate(ground.transform, glm::vec3(0, 3, 0));
    ground.material.color = glm::vec4(.6f, .6f, .6f, 1);
    scene.planes.push_back(ground);

    Camera camera;
    camera.projection = glm::perspective(45.f, 4.f / 3.f, .1f, 100.f);
    camera.transform = glm::lookAt(glm::vec3(-3, -1.2f, -3), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
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
