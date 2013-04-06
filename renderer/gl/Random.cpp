// Copyright (C) 2012 Sami Kyöstilä

#include "Random.h"

using namespace gl;

void Random::writeRandomNumberGenerator(std::ostringstream& s) const
{
    s << "#define M_PI 3.1415926535897932384626433832795\n"
         "#define M_1_PI 0.31830988618379067154\n"
         "\n";

    s << "uniform vec2 randomSeed;\n"
         "\n"
         "float random(vec2 position)\n"
         "{\n"
         "    return fract(sin(dot(randomSeed + position.xy, vec2(12.9898, 78.233))) * 43758.5453);\n"
         "}\n"
         "\n";

    s << "struct RandomVec3 {\n"
         "    vec3 value;\n"
         "    float probability;\n"
         "};\n"
         "\n";

    s << "RandomVec3 generateCosineHemispherical(vec3 seed)\n"
         "{\n"
         "    float u = random(seed.xy);\n"
         "    float v = random(seed.yz);\n"
         "    float r = sqrt(u);\n"
         "    float phi = v * 2.0 * M_PI;\n"
         "    float x = r * cos(phi);\n"
         "    float y = r * sin(phi);\n"
         "    float z = sqrt(max(0.0, 1.0 - u));\n"
         "    RandomVec3 result;\n"
         "    result.value = vec3(x, y, z);\n"
         "    result.probability = z * M_1_PI;\n"
         "    return result;\n"
         "};\n"
         "\n";

    s << "RandomVec3 generatePhong(vec3 seed, float exponent)\n"
         "{\n"
         "    float u = random(seed.xy);\n"
         "    float v = random(seed.yz);\n"
         "    float a = acos(pow(max(u, 0.000001), 1.0 / (exponent + 1.0)));\n"
         "    float phi = 2.0 * M_PI * v;\n"
         "\n"
         "    RandomVec3 result;\n"
         "    result.value = vec3(sin(a) * cos(phi),\n"
         "                        sin(a) * sin(phi),\n"
         "                        cos(a));\n"
         "    result.probability = (exponent + 1.0) / (2.0 * M_PI) * pow(cos(a), exponent);\n"
         "    return result;\n"
         "};\n"
         "\n";

    s << "struct RandomBool {\n"
         "    bool value;\n"
         "    float probability;\n"
         "};\n"
         "\n";

    s << "RandomBool flipCoin(vec2 seed, float probability)\n"
         "{\n"
         "    float r = random(seed);\n"
         "    RandomBool result;\n"
         "    result.value = probability > 0.0 && r <= probability;\n"
         "    result.probability = result.value ? probability : (1.0 - probability);\n"
         "    return result;\n"
         "};\n"
         "\n";
}

void Random::setRandomNumberGeneratorUniforms(GLuint program) const
{
    glUniform2f(uniform(program, "randomSeed"), drand48(), drand48());
}
