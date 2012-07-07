#include "Renderer.h"
#include "scene/Scene.h"

#include <glm/gtc/matrix_transform.hpp>

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

int main(int argc, char** argv)
{
    scene::Scene scene;
    Renderer renderer;

    buildTestScene(scene);
}
