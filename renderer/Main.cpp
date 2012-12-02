// Copyright (C) 2012 Sami Kyöstilä
#include "Scheduler.h"
#include "cpu/Scheduler.h"
#include "gl/Scheduler.h"
#include "scene/Parser.h"
#include "scene/Scene.h"
#include "Image.h"
#include "Preview.h"

#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

void buildTestScene(scene::Scene& scene)
{
    using namespace scene;

    glm::vec4 colors[] =
    {
        glm::vec4(1, 1, 1, 1),
        glm::vec4(.8f, .1f, .1f, 1),
        glm::vec4(.1f, .8f, .1f, 1),
        glm::vec4(.1f, .1f, .8f, 1),
    };

    for (int i = 0; i < 4; i++)
    {
        Sphere sphere;
        sphere.radius = 1.f;
        sphere.material.ambient = colors[i % 4] * 0.1f;
        sphere.material.diffuse = colors[i % 4];
        if (i % 4 == 1)
            sphere.material.specularExponent = 20;
        if (i % 4 == 0)
        {
            sphere.material.transparency = glm::vec4(0.9f);
            sphere.material.refractiveIndex = 1.5f;
        }
        sphere.transform = glm::translate(sphere.transform, glm::vec3(i * 3 - 2, 0, i * .5f));
        scene.spheres.push_back(sphere);
    }

    Sphere sphere;
    sphere.radius = .3f;
    sphere.material.emission = glm::vec4(1, 1, 1, 0) * 8;
    sphere.transform = glm::translate(sphere.transform, glm::vec3(0, -1.5, 2));
    scene.spheres.push_back(sphere);

    Plane ground;
    ground.transform = glm::translate(ground.transform, glm::vec3(0, 1, 0));
    ground.material.diffuse = glm::vec4(.4f, .4f, .4f, 1);
    ground.material.ambient = ground.material.diffuse * 0.05f;
    scene.planes.push_back(ground);

    Plane wall;
    wall.transform = glm::rotate(wall.transform, -90.f, glm::vec3(1, 0, 0));
    wall.transform = glm::translate(wall.transform, glm::vec3(0, 2, 0));
    wall.material.diffuse = glm::vec4(1.f, 1.f, 1.f, 1);
    wall.material.ambient = ground.material.diffuse * 0.05f;
    scene.planes.push_back(wall);

    Plane wall2;
    wall2.transform = glm::rotate(wall2.transform, -90.f, glm::vec3(0, 0, 1));
    wall2.transform = glm::translate(wall2.transform, glm::vec3(0, 10, 0));
    wall2.material.diffuse = glm::vec4(1.f, 1.f, 1.f, 1);
    wall2.material.ambient = ground.material.diffuse * 0.05f;
    scene.planes.push_back(wall2);

    Plane wall3;
    wall3.transform = glm::rotate(wall3.transform, 90.f, glm::vec3(0, 0, 1));
    wall3.transform = glm::translate(wall3.transform, glm::vec3(0, 8, 0));
    wall3.material.diffuse = glm::vec4(1.f, 1.f, 1.f, 1);
    wall3.material.ambient = ground.material.diffuse * 0.05f;
    scene.planes.push_back(wall3);

    Plane wall4;
    wall4.transform = glm::rotate(wall4.transform, 90.f, glm::vec3(1, 0, 0));
    wall4.transform = glm::translate(wall4.transform, glm::vec3(0, 6, 0));
    wall4.material.diffuse = glm::vec4(1.f, 1.f, 1.f, 1);
    wall4.material.ambient = ground.material.diffuse * 0.05f;
    scene.planes.push_back(wall4);

    Plane ceiling;
    ceiling.transform = glm::rotate(ceiling.transform, 180.f, glm::vec3(1, 0, 0));
    ceiling.transform = glm::translate(ceiling.transform, glm::vec3(0, 2, 0));
    ceiling.material.diffuse = glm::vec4(1.f, 1.f, 1.f, 1);
    ceiling.material.ambient = ground.material.diffuse * 0.05f;
    scene.planes.push_back(ceiling);

    Camera camera;
    camera.projection = glm::perspective(45.f, 4.f / 3.f, .1f, 100.f);
    camera.transform = glm::lookAt(glm::vec3(-6, -0.8f, 4), glm::vec3(0, 0, 0), glm::vec3(0, -1, 0));
    scene.camera = camera;

    scene.backgroundColor = glm::vec4(.0f, .0f, .0f, 1);
}

int main(int argc, char** argv)
{
    std::vector<std::string> args(&argv[0], &argv[argc]);
    std::string rendererName = "cpu";

    int width = 640;
    int height = 480;
    for (size_t i = 1; i < args.size() - 1; i++) {
        bool hasMoreArgs = i < args.size() - 2;
        if (args[i] == "--help") {
            printf("Usage: %s OPTIONS SCENE\n\n"
                   "Options:\n"
                   "    -w SIZE    Image width (640)\n"
                   "    -h SIZE    Image height (480)\n"
                   "    -r NAME    Renderer (cpu, gl)\n", args[0].c_str());
            return 1;
        } else if (args[i] == "-w" && hasMoreArgs) {
            width = atoi(args[++i].c_str());
        } else if (args[i] == "-h" && hasMoreArgs) {
            height = atoi(args[++i].c_str());
        } else if (args[i] == "-r" && hasMoreArgs) {
            rendererName = args[++i];
        }
    }

    scene::Scene scene;
    if (argc == 1)
        buildTestScene(scene);
    else if (!scene::Parser::load(scene, args[args.size() - 1],
                                  static_cast<float>(width) / height)) {
        std::cerr << "Failed to parse scene from " << args[args.size() - 1] << std::endl;
        return 1;
    }

    std::unique_ptr<Image> image(new Image(width, height));
    std::unique_ptr<Preview> preview(Preview::create(image.get(), true));
    std::unique_ptr<Scheduler> scheduler;

    if (rendererName == "cpu") {
        scheduler.reset(new cpu::Scheduler(scene, image.get(), preview.get()));
    } else if (rendererName == "gl") {
        scheduler.reset(new gl::Scheduler(scene, image.get(), preview.get()));
    } else {
        std::cerr << "Unknown renderer: " << rendererName << std::endl;
        return 1;
    }

    scheduler->run();
    image->save("out.png");
}
