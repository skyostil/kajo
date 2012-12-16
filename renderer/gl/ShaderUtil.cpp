// Copyright (C) 2012 Sami Kyöstilä
#include "ShaderUtil.h"

namespace gl
{

void writeFloat(std::ostringstream& s, float value)
{
    std::string v = std::to_string(value);
    s << v;
    if (v.find('.') == std::string::npos)
        s << ".0";
}

void writeMatrix(std::ostringstream& s, const glm::mat4& m)
{
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            writeFloat(s, m[i][j]);
            if (i != 3 || j != 3)
                s << ", ";
        }
    }
}

void writeVec2(std::ostringstream& s, const glm::vec2& v)
{
    writeFloat(s, v.x);
    s << ", ";
    writeFloat(s, v.y);
}

void writeVec3(std::ostringstream& s, const glm::vec3& v)
{
    writeFloat(s, v.x);
    s << ", ";
    writeFloat(s, v.y);
    s << ", ";
    writeFloat(s, v.z);
}

void writeVec4(std::ostringstream& s, const glm::vec4& v)
{
    writeFloat(s, v.x);
    s << ", ";
    writeFloat(s, v.y);
    s << ", ";
    writeFloat(s, v.z);
    s << ", ";
    writeFloat(s, v.w);
}

}
