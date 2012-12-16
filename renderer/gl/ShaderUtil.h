// Copyright (C) 2012 Sami Kyöstilä
#ifndef GL_SHADERUTIL_H
#define GL_SHADERUTIL_H

#include <glm/glm.hpp>
#include <sstream>

namespace gl
{

void writeFloat(std::ostringstream& s, float value);
void writeMatrix(std::ostringstream& s, const glm::mat4& m);
void writeVec2(std::ostringstream& s, const glm::vec2& v);
void writeVec3(std::ostringstream& s, const glm::vec3& v);
void writeVec4(std::ostringstream& s, const glm::vec4& v);

}

#endif
