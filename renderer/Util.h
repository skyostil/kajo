// Copyright (C) 2012 Sami Kyöstilä
#ifndef UTIL_H
#define UTIL_H

#include <glm/glm.hpp>
#include <sstream>

void dump(const glm::vec3& v);
void dump(const glm::vec4& v);

template <typename T>
void formatSI(std::ostringstream& result, T n, const char* units)
{
    result.precision(2);
    result.setf(std::ios::fixed, std::ios::floatfield);

    if (n >= 1000 * 1000 * 1000LL)
        result << n / (1000.f * 1000 * 1000) << " G";
    else if (n >= 1000 * 1000)
        result << n / (1000.f * 1000) << " M";
    else if (n >= 1000)
        result << n / (1000.f) << " K";
    else
        result << n << " ";

    result << units;
}

#endif
