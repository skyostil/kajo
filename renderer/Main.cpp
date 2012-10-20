// Copyright (C) 2012 Sami Kyöstilä
#include "Image.h"
#include "Preview.h"
#include "Queue.h"
#include "Raytracer.h"
#include "Renderer.h"
#include "Shader.h"
#include "scene/Scene.h"

#include <JSON.h>
#include <algorithm>
#include <fstream>
#include <future>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <vector>

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
    sphere.material.emission = glm::vec4(1, 1, 1, 0) * 8;
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
    scene.camera = camera;

    scene.backgroundColor = glm::vec4(.0f, .0f, .0f, 1);
}

static int hexToInt(wchar_t digit)
{
    if (digit >= L'0' && digit <= L'9')
        return (digit - L'0');
    else if (digit >= L'A' && digit <= L'F')
        return (digit - L'A' + 10);
    else if (digit >= L'a' && digit <= L'f')
        return (digit - L'a' + 10);
    return 0;
}

static glm::vec4 parseVec4(std::wistream& ss)
{
    glm::vec4 result;
    ss >> result.x;
    if (ss.peek() == L',')
        ss.ignore();
    ss >> result.y;
    if (ss.peek() == L',')
        ss.ignore();
    ss >> result.z;
    if (ss.peek() == L',')
        ss.ignore();
    ss >> result.w;
    if (ss.peek() == L',')
        ss.ignore();
    return result;
}

static glm::vec4 parseVec4(const std::wstring& s)
{
    std::wstringstream ss(s);
    return parseVec4(ss);
}

static glm::vec3 parseVec3(std::wistream& ss)
{
    glm::vec3 result;
    ss >> result.x;
    if (ss.peek() == L',')
        ss.ignore();
    ss >> result.y;
    if (ss.peek() == L',')
        ss.ignore();
    ss >> result.z;
    if (ss.peek() == L',')
        ss.ignore();
    return result;
}

static glm::vec3 parseVec3(const std::wstring& s)
{
    std::wstringstream ss(s);
    return parseVec3(ss);
}

static glm::vec4 parseColor(const std::wstring& value)
{
    if (value.size() == 4 && value[0] == L'#')
        return glm::vec4(hexToInt(value[1]) / 15.f, hexToInt(value[2]) / 15.f, hexToInt(value[3]) / 15.f, 1);
    else if (value.size() == 7 && value[0] == L'#')
    {
        return glm::vec4(
                (hexToInt(value[1]) * 16 + hexToInt(value[2])) / 255.f,
                (hexToInt(value[3]) * 16 + hexToInt(value[4])) / 255.f,
                (hexToInt(value[5]) * 16 + hexToInt(value[6])) / 255.f, 1);
    }
    else if (value.size() >= 6 && value.find(L"rgba(") == 0)
        return parseVec4(value.substr(5));
    else if (value.size() >= 5 && value.find(L"rgb(") == 0)
        return glm::vec4(parseVec3(value.substr(4)), 1);
    return glm::vec4();
}

static bool expect(std::wistream& s, const std::wstring& expected)
{
    std::wstring word;
    s >> word;
    return word == expected;
}

static glm::mat4 parseTransform(const std::wstring& value)
{
    std::wstringstream ss(value);
    std::wstring command;

    glm::mat4 result;
    while (std::getline(ss, command, L'('))
    {
        if (command == L"lookat")
        {
            const int paramCount = 9;
            float params[paramCount] = {0};
            for (int i = 0; i < paramCount; ++i)
            {
                if (!(ss >> params[i]))
                    break;
                if (ss.peek() == L',')
                    ss.ignore();
            }
            expect(ss, L")");
            result *= glm::lookAt(
                    glm::vec3(params[0], params[1], params[2]),
                    glm::vec3(params[3], params[4], params[5]),
                    glm::vec3(params[6], params[7], params[8]));
        }
        else if (command == L"translate")
        {
            glm::vec3 offset = parseVec3(ss);
            expect(ss, L")");
            result = glm::translate(result, offset);
        }
        else if (command == L"scale")
        {
            glm::vec3 scale = parseVec3(ss);
            expect(ss, L")");
            result = glm::scale(result, scale);
        }
        else if (command == L"rotate")
        {
            glm::vec4 rotation = parseVec4(ss);
            expect(ss, L")");
            result = glm::rotate(result, rotation.x, glm::vec3(rotation.y, rotation.z, rotation.w));
        }
        while (ss.peek() == L' ')
            ss.ignore();
    }
    return result;
}

static scene::Camera parseCamera(JSONObject data)
{
    scene::Camera camera;
    if (data.find(L"projection") != data.end())
    {
        std::wstring projection = data[L"projection"]->AsString();
        if (projection.find(L"perspective(") == 0)
        {
            std::wstringstream ss(projection.substr(12));
            glm::vec4 p = parseVec4(ss);
            camera.projection = glm::perspective(p.x, p.y, p.z, p.w);
        }
    }
    if (data.find(L"transform") != data.end())
        camera.transform = parseTransform(data[L"transform"]->AsString());
    return camera;
}

static void parseObjects(scene::Scene& scene, JSONArray data)
{
    for (size_t i = 0; i < data.size(); i++)
    {
        JSONObject objectData = data[i]->AsObject();
        if (objectData.find(L"type") == objectData.end())
            continue;

        scene::Material material;
        if (objectData.find(L"diffuse") != objectData.end())
            material.diffuse = parseColor(objectData[L"diffuse"]->AsString());
        if (objectData.find(L"emission") != objectData.end())
            material.emission = parseColor(objectData[L"emission"]->AsString());

        glm::mat4 transform;
        if (objectData.find(L"transform") != objectData.end())
            transform = parseTransform(objectData[L"transform"]->AsString());

        if (objectData[L"type"]->AsString() == L"sphere")
        {
            scene::Sphere sphere;
            sphere.radius = objectData[L"radius"]->AsNumber();
            sphere.material = material;
            sphere.transform = transform;
            scene.spheres.push_back(sphere);
        }
        else if (objectData[L"type"]->AsString() == L"plane")
        {
            scene::Plane plane;
            plane.material = material;
            plane.transform = transform;
            scene.planes.push_back(plane);
        }
    }
}

bool loadScene(scene::Scene& scene, const std::string& fileName)
{
    std::stringstream source;
    source << std::ifstream(fileName).rdbuf();

    std::unique_ptr<JSONValue> data(JSON::Parse(source.str().c_str()));
    if (!data || !data->IsObject())
    {
        std::cerr << "Failed to parse scene from " << fileName << std::endl;
        return false;
    }

    JSONObject sceneData = data->AsObject();
    if (sceneData.find(L"background") != sceneData.end())
        scene.backgroundColor = parseColor(sceneData[L"background"]->AsString());
    if (sceneData.find(L"camera") != sceneData.end())
        scene.camera = parseCamera(sceneData[L"camera"]->AsObject());
    if (sceneData.find(L"objects") != sceneData.end())
        parseObjects(scene, sceneData[L"objects"]->AsArray());

    return true;
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

void createTasks(Image& image, Renderer& renderer, RenderTasks& tasks)
{
    int slice = (image.height + 1) / cpuCount();
    for (int y = 0; y < image.height; y += slice)
    {
        auto task = std::async(std::launch::async, [=, &renderer, &image] {
            renderer.render(image, 0, y, image.width, slice);
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

void render(Image& image, scene::Scene& scene)
{
    Renderer renderer(&scene);
    std::unique_ptr<Preview> preview(Preview::create(&image));
    bool done = false;

    Queue<RenderUpdate> updateQueue;
    renderer.setObserver([&updateQueue, &done] (int pass, int samples, int xOffset, int yOffset, int width, int height) {
        std::thread::id threadId = std::this_thread::get_id();
        updateQueue.push(RenderUpdate{threadId, pass, samples, xOffset, yOffset, width, height});
        return !done;
    });

    RenderTasks tasks;
    createTasks(image, renderer, tasks);

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

    if (argc == 1)
        buildTestScene(scene);
    else if (!loadScene(scene, argv[1]))
        return 1;

    Image image(640, 480);
    render(image, scene);
    image.save("out.png");
}
