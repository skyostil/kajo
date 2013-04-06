// Copyright (C) 2012 Sami Kyöstilä
#ifndef CPU_BSDF_H
#define CPU_BSDF_H

#include <glm/glm.hpp>

namespace cpu
{

class SurfacePoint;
class Random;

template <typename T> class RandomValue;

class BSDF
{
public:
    BSDF(const SurfacePoint*);

    virtual RandomValue<glm::vec3> generateSample(Random& random) const = 0;
    virtual glm::vec4 evaluateSample(const glm::vec3& direction) const = 0;
    virtual float sampleProbability(const glm::vec3& direction) const = 0;

protected:
    const SurfacePoint* m_surfacePoint;
};

class LambertBSDF: public BSDF
{
public:
    LambertBSDF(const SurfacePoint*, const glm::vec4& color);

    RandomValue<glm::vec3> generateSample(Random& random) const override;
    glm::vec4 evaluateSample(const glm::vec3& direction) const override;
    float sampleProbability(const glm::vec3& direction) const override;
private:
    glm::vec4 m_color;
};

class PhongBSDF: public BSDF
{
public:
    PhongBSDF(const SurfacePoint*, const glm::vec4& color, float exponent);

    RandomValue<glm::vec3> generateSample(Random& random) const override;
    glm::vec4 evaluateSample(const glm::vec3& direction) const override;
    float sampleProbability(const glm::vec3& direction) const override;
private:
    glm::vec4 m_color;
    float m_exponent;
};

class IdealReflectorBSDF: public BSDF
{
public:
    IdealReflectorBSDF(const SurfacePoint*, const glm::vec4& color);

    RandomValue<glm::vec3> generateSample(Random& random) const override;
    glm::vec4 evaluateSample(const glm::vec3& direction) const override;
    float sampleProbability(const glm::vec3& direction) const override;
private:
    glm::vec4 m_color;
};

class IdealTransmissionBSDF: public BSDF
{
public:
    IdealTransmissionBSDF(const SurfacePoint*, const glm::vec4& color,
                          float refractiveIndex);

    RandomValue<glm::vec3> generateSample(Random& random) const override;
    glm::vec4 evaluateSample(const glm::vec3& direction) const override;
    float sampleProbability(const glm::vec3& direction) const override;

    const glm::vec3& shadingNormal() const;
private:
    glm::vec4 m_color;
    float m_refractiveIndex;
};


}

#endif // BSDF_H
