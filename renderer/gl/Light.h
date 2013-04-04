// Copyright (C) 2013 Sami Kyöstilä
#ifndef GL_LIGHT_H
#define GL_LIGHT_H

#include <glm/glm.hpp>
#include <sstream>

namespace gl
{

class Sphere;

class SphericalLight
{
public:
    SphericalLight(const Sphere*);

    static void writeCommon(std::ostringstream& s);
    void writeSampleGenerator(std::ostringstream& s, const std::string& name) const;
    void writeSampleEvaluator(std::ostringstream& s, const std::string& name) const;
    void writeSampleProbability(std::ostringstream& s, const std::string& name) const;

private:
    const Sphere* m_sphere;
};

}

#endif // LIGHT_H
