// Copyright (C) 2012 Sami Kyöstilä
#ifndef SHADER_H
#define SHADER_H

#include "PrecalculatedScene.h"

#include <glm/glm.hpp>
#include <memory>
#include <vector>

class Ray;
class Raytracer;
class Surface;

namespace scene
{
    class Material;
    class Sphere;
    class Plane;
    class PointLight;
    class Scene;
}

class Shader
{
public:
    Shader(scene::Scene* scene, Raytracer* raytracer);

    enum ObjectSamplingScheme
    {
        SampleNonEmissiveObjects,
        SampleAllObjects,
    };

    glm::vec4 shade(const Ray& ray, int depth = 0, ObjectSamplingScheme = SampleAllObjects) const;

private:
    template <typename ObjectType>
    void applyAllEmissiveObjects(const std::vector<ObjectType>& lights,
                                 const TransformDataList& transformDataList,
                                 const Ray& ray, glm::vec4& color) const;

    void applyEmissiveObject(const scene::Sphere& sphere, const TransformData& data,
                             const Ray& ray, glm::vec4& color) const;

    glm::vec4 sampleBRDF(const Ray& ray, glm::vec3& direction, int depth) const;
    template <typename ObjectType>
    glm::vec4 sampleObjects(const std::vector<ObjectType>& lights,
                            const TransformDataList& transformDataList,
                            const Ray& ray) const;
    glm::vec4 sampleObject(const scene::Sphere& sphere, const TransformData& data,
                           const Ray& ray, glm::vec3& direction) const;

    float pdfForBRDF(const Ray& ray, const glm::vec3& direction) const;
    template <typename ObjectType>
    float pdfForObjects(const std::vector<ObjectType>& lights,
                        const TransformDataList& transformDataList,
                        const Ray& ray, const glm::vec3& direction) const;
    float pdfForObject(const scene::Sphere& sphere, const TransformData& data,
                       const Ray& ray, const glm::vec3& direction) const;

    scene::Scene* m_scene;
    Raytracer* m_raytracer;
};

#endif
