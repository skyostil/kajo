// Copyright (C) 2012 Sami Kyöstilä
#ifndef PARSER_H
#define PARSER_H

#include <string>

namespace scene
{

class Scene;

class Parser
{
public:
    static bool load(Scene& scene, const std::string& fileName, float aspectRatio);
};

}

#endif // PARSER_H
