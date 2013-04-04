// Copyright (C) 2013 Sami Kyöstilä
#ifndef GL_BSDF_H
#define GL_BSDF_H

#include <sstream>

namespace gl
{

class BSDF
{
public:
    static void writeBSDFFunctions(std::ostringstream& s);
};

}

#endif // GL_BSDF_H
