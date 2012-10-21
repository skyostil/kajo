// Copyright (C) 2012 Sami Kyöstilä
#include "Parser.h"
#include "Scene.h"

#include <JSON.h>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <sstream>

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
        if (objectData.find(L"specular") != objectData.end())
            material.specular = parseColor(objectData[L"specular"]->AsString());
        if (objectData.find(L"specularExponent") != objectData.end())
            material.specularExponent = objectData[L"specularExponent"]->AsNumber();
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

bool scene::Parser::load(Scene& scene, const std::string& fileName)
{
    std::stringstream source;
    source << std::ifstream(fileName).rdbuf();

    std::unique_ptr<JSONValue> data(JSON::Parse(source.str().c_str()));
    if (!data || !data->IsObject())
        return false;

    JSONObject sceneData = data->AsObject();
    if (sceneData.find(L"background") != sceneData.end())
        scene.backgroundColor = parseColor(sceneData[L"background"]->AsString());
    if (sceneData.find(L"camera") != sceneData.end())
        scene.camera = parseCamera(sceneData[L"camera"]->AsObject());
    if (sceneData.find(L"objects") != sceneData.end())
        parseObjects(scene, sceneData[L"objects"]->AsArray());

    return true;
}
