// Copyright (C) 2012 Sami Kyöstilä
#ifndef GL_RAYTRACER_H
#define GL_RAYTRACER_H

#include "renderer/GLHelpers.h"

#include <sstream>

namespace gl
{

class Scene;

class Raytracer
{
public:
    Raytracer(Scene* scene);

    void writeRayGenerator(std::ostringstream& s) const;
    void setRayGeneratorUniforms(GLuint program) const;

private:

    Scene* m_scene;
};

}

#endif
