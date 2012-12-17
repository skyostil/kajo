// Copyright (C) 2012 Sami Kyöstilä

#include "Random.h"

using namespace gl;

void Random::writeRandomNumberGenerator(std::ostringstream& s) const
{
    s << "uniform vec2 randomSeed;\n"
         "\n"
         "float random(vec2 position)\n"
         "{\n"
         "    return fract(sin(dot(randomSeed + position.xy, vec2(12.9898, 78.233))) * 43758.5453);\n"
         "}\n"
         "\n";
}

void Random::setRandomNumberGeneratorUniforms(GLuint program) const
{
    glUniform2f(uniform(program, "randomSeed"), drand48(), drand48());
}
