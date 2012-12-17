// Copyright (C) 2012 Sami Kyöstilä
#ifndef GL_SHADER_H
#define GL_SHADER_H

#include "renderer/GLHelpers.h"

#include <sstream>

namespace gl
{

class Scene;
class Raytracer;
class Random;

class SurfaceShader
{
public:
    SurfaceShader(Scene* scene, Raytracer* raytracer, Random* random);

    void writeSurfaceShader(std::ostringstream& s) const;
    void setSurfaceShaderUniforms(GLuint program) const;

private:

    Scene* m_scene;
    Raytracer* m_raytracer;
    Random* m_random;
};

}

#endif
