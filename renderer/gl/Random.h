// Copyright (C) 2012 Sami Kyöstilä
#ifndef GL_RANDOM_H
#define GL_RANDOM_H

#include "renderer/GLHelpers.h"

#include <sstream>

namespace gl
{

class Random
{
public:
    void writeRandomNumberGenerator(std::ostringstream& s) const;
    void setRandomNumberGeneratorUniforms(GLuint program) const;
};

}

#endif
