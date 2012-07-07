#include "Renderer.h"
#include "scene/Scene.h"

#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <lodepng.h>

void buildTestScene(scene::Scene& scene)
{
    using namespace scene;

    Sphere sphere;
    sphere.radius = 1;
    sphere.material.color = glm::vec3(0, 1, 0);

    Camera camera;
    camera.projection = glm::perspective(45.f, 4.f / 3.f, .1f, 100.f);
    camera.view = glm::translate(camera.view, glm::vec3(0, 0, -3.f));
    scene.camera = camera;

    scene.spheres.push_back(sphere);
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
