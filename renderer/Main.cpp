// Copyright (C) 2012 Sami Kyöstilä
#include "Preview.h"
#include "Queue.h"
#include "Raytracer.h"
#include "Renderer.h"
#include "Shader.h"
#include "Surface.h"
#include "scene/Scene.h"

#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <unistd.h>
#include <future>
#include <vector>
#include <algorithm>

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
        if (i % 4 == 3)
            sphere.material.reflectivity = 1.f;
        if (i % 4 == 0)
        {
            sphere.material.transparency = 0.9f;
            sphere.material.reflectivity = 0.2f;
            sphere.material.refractiveIndex = 1.5f;
        }
        sphere.transform = glm::translate(sphere.transform, glm::vec3(i * 3 - 2, 0, i * .5f));
        scene.spheres.push_back(sphere);
    }

    Sphere sphere;
    sphere.radius = .3f;
    sphere.material.emission = glm::vec4(1, 1, 1, 0);
    sphere.transform = glm::translate(sphere.transform, glm::vec3(0, -1.5, 2));
    scene.spheres.push_back(sphere);

    Plane ground;
    ground.transform = glm::translate(ground.transform, glm::vec3(0, 1, 0));
    ground.material.diffuse = glm::vec4(.4f, .4f, .4f, 1);
    ground.material.ambient = ground.material.diffuse * 0.05f;
    ground.material.reflectivity = 0.5f * 0;
    ground.material.checkerboard = true;
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
    //camera.transform = glm::lookAt(glm::vec3(-10, -2, 1), glm::vec3(0, 0, 0), glm::vec3(0, -1, 0));
    scene.camera = camera;

    scene.backgroundColor = glm::vec4(.0f, .0f, .0f, 1);
}

int cpuCount()
{
    return sysconf(_SC_NPROCESSORS_ONLN);
}

struct RenderUpdate
{
    std::thread::id threadId;
    int pass;
    int samples;
    int xOffset, yOffset, width, height;
};

typedef std::vector<std::future<void>> RenderTasks;

void createTasks(Surface& surface, Renderer& renderer, RenderTasks& tasks)
{
    int slice = (surface.height + 1) / cpuCount();
    for (int y = 0; y < surface.height; y += slice)
    {
        auto task = std::async(std::launch::async, [=, &renderer, &surface] {
            renderer.render(surface, 0, y, surface.width, slice);
        });
        tasks.push_back(std::move(task));
    }
}

void joinTasks(RenderTasks& tasks)
{
    while (tasks.size())
    {
        tasks.back().wait();
        tasks.pop_back();
    }
}

void render(Surface& surface, scene::Scene& scene)
{
    Renderer renderer(&scene);
    std::unique_ptr<Preview> preview(Preview::create(&surface));
    bool done = false;

    Queue<RenderUpdate> updateQueue;
    renderer.setObserver([&updateQueue, &done] (int pass, int samples, int xOffset, int yOffset, int width, int height) {
        std::thread::id threadId = std::this_thread::get_id();
        updateQueue.push(RenderUpdate{threadId, pass, samples, xOffset, yOffset, width, height});
        return !done;
    });

    RenderTasks tasks;
    createTasks(surface, renderer, tasks);

    while (preview->processEvents())
    {
        RenderUpdate update;
        if (updateQueue.pop(update, std::chrono::milliseconds(500)))
            preview->update(update.threadId, update.pass, update.samples,
                            update.xOffset, update.yOffset,
                            update.width, update.height);
    }

    done = true;
    joinTasks(tasks);
}

int main(int argc, char** argv)
{
    scene::Scene scene;

    buildTestScene(scene);

    Surface surface(640, 480);
    render(surface, scene);
    surface.save("out.png");
}
