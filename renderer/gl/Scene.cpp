// Copyright (C) 2012 Sami Kyöstilä
#include "Scene.h"
#include "scene/Scene.h"

#include <algorithm>

using namespace gl;

Transform::Transform(const glm::mat4 matrix):
    matrix(matrix),
    invMatrix(glm::inverse(matrix)),
    determinant(glm::determinant(matrix))
{
}

Sphere::Sphere(const scene::Sphere& sphere):
    transform(sphere.transform),
    material(sphere.material),
    radius(sphere.radius)
{
}

Plane::Plane(const scene::Plane& plane):
    transform(plane.transform),
    material(plane.material)
{
}

Scene::Scene(const scene::Scene& scene):
    backgroundColor(scene.backgroundColor),
    camera(scene.camera)
{
    for (const scene::Sphere& sphere: scene.spheres)
        spheres.push_back(Sphere(sphere));
    for (const scene::Plane& plane: scene.planes)
        planes.push_back(Plane(plane));
}
