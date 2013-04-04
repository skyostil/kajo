#include "Light.h"
#include "Scene.h"
#include "ShaderUtil.h"

using namespace gl;

SphericalLight::SphericalLight(const Sphere* sphere):
    m_sphere(sphere)
{
}

void SphericalLight::writeCommon(std::ostringstream& s)
{
    s << "float solidAngle(float radius, vec3 lightPos, vec3 surfacePos)\n"
         "{\n"
         "    float dist = length(lightPos - surfacePos);\n"
         "    if (dist < radius)\n"
         "        return 4.0 * M_PI;\n"
         "    return 2.0 * M_PI * (1.0 / cos(asin(radius / dist)));\n"
         "}\n";
}

void SphericalLight::writeSampleGenerator(std::ostringstream& s, const std::string& name) const
{
    s << "RandomVec3 " << name << "(vec3 surfacePos)\n"
         "{\n"
         "    float s1 = random(surfacePos.xy);\n"
         "    float s2 = random(surfacePos.yz);\n"
         "    float s3 = random(surfacePos.zx);\n"
         "\n";

    glm::vec3 lightPos(m_sphere->transform.matrix * glm::vec4(0, 0, 0, 1));
    s << "    vec3 lightPos = vec3(";
    writeVec3(s, lightPos);
    s << ");\n";

    s << "    float radius = ";
    writeFloat(s, m_sphere->radius);
    s << ";\n";

    s << "    float radius2 = ";
    writeFloat(s, m_sphere->radius * m_sphere->radius);
    s << ";\n";

    s << "    float x = radius * sqrt(s1) * cos(2.0 * M_PI * s2);\n"
         "    float y = radius * sqrt(s1) * sin(2.0 * M_PI * s2);\n"
         "    float z = sqrt(radius2 - x * x - y * y) * sin(M_PI * (s3 - .5));\n"
         "\n"
         "    RandomVec3 result;\n"
         "    result.value = normalize(lightPos + vec3(x, y, z) - surfacePos);\n"
         "    result.probability = 1.0 / solidAngle(radius, lightPos, surfacePos);\n"
         "    return result;\n"
         "}\n";
}

void SphericalLight::writeSampleEvaluator(std::ostringstream& s, const std::string& name) const
{
    s << "vec4 " << name << "(vec3 surfacePos, vec3 direction)\n";
    s << "{\n";
    s << "    return vec4(";
    writeVec4(s, m_sphere->material.material.emission);
    s << ");\n";
    s << "}\n";
}

void SphericalLight::writeSampleProbability(std::ostringstream& s, const std::string& name) const
{
    s << "float " << name << "(vec3 surfacePos, vec3 direction)\n";
    s << "{\n";

    glm::vec3 lightPos(m_sphere->transform.matrix * glm::vec4(0, 0, 0, 1));
    s << "    vec3 lightPos = vec3(";
    writeVec3(s, lightPos);
    s << ");\n";

    s << "    float radius = ";
    writeFloat(s, m_sphere->radius);
    s << ";\n";

    s << "    return solidAngle(radius, lightPos, surfacePos);\n";
    s << "}\n";
}
