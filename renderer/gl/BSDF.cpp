// Copyright (C) 2012 Sami Kyöstilä
#include "BSDF.h"

using namespace gl;

void BSDF::writeBSDFFunctions(std::ostringstream& s)
{
    // Lambert
    s << "RandomVec3 generateLambertSample(SurfacePoint surfacePoint, Material material)\n"
         "{\n"
         "    vec3 seed = surfacePoint.position + surfacePoint.view;\n"
         "    RandomVec3 result = generateCosineHemispherical(seed);\n"
         "    result.value =\n"
         "        surfacePoint.tangent * result.value.x +\n"
         "        surfacePoint.binormal * result.value.y +\n"
         "        surfacePoint.normal * result.value.z;\n"
         "    return result;\n"
         "}\n"
         "\n";

    s << "vec4 evaluateLambertSample(SurfacePoint surfacePoint, Material material, vec3 direction)\n"
         "{\n"
         "    return material.diffuse * M_1_PI;\n"
         "}\n"
         "\n";

    s << "float lambertSampleProbability(SurfacePoint surfacePoint, Material material, vec3 direction)\n"
         "{\n"
         "    float cos_theta = dot(direction, surfacePoint.normal);\n"
         "    return M_1_PI * cos_theta;\n"
         "}\n"
         "\n";

    // Phong
    s << "RandomVec3 generatePhongSample(SurfacePoint surfacePoint, Material material)\n"
         "{\n"
         "    vec3 seed = surfacePoint.position + surfacePoint.view;\n"
         "    RandomVec3 result = generatePhong(seed, material.specularExponent);\n"
         "    vec3 reflection = reflect(surfacePoint.view, surfacePoint.normal);\n"
         "    vec3 r = vec3(0.0, 0.0, 1.0);\n"
         "    vec3 u = normalize(cross(r, reflection));\n"
         "    vec3 v = cross(u, reflection);\n"
         "    result.value = mat3(u, v, reflection) * result.value;\n"
         "    return result;\n"
         "}\n"
         "\n";

    s << "vec4 evaluatePhongSample(SurfacePoint surfacePoint, Material material, vec3 direction)\n"
         "{\n"
         "    vec3 reflection = reflect(surfacePoint.view, surfacePoint.normal);\n"
         "    float cos_a = max(0.0, dot(reflection, direction));\n"
         "    return (material.specularExponent + 1.0) / (2.0 * M_PI) * material.specular * pow(cos_a, material.specularExponent);\n"
         "}\n"
         "\n";

    s << "float phongSampleProbability(SurfacePoint surfacePoint, Material material, vec3 direction)\n"
         "{\n"
         "    vec3 reflection = reflect(surfacePoint.view, surfacePoint.normal);\n"
         "    float cos_a = max(0.0, dot(reflection, direction));\n"
         "    return (material.specularExponent + 1.0) / (2.0 * M_PI) * pow(cos_a, material.specularExponent);\n"
         "}\n"
         "\n";

    // Ideal reflector
    s << "RandomVec3 generateIdealReflectorSample(SurfacePoint surfacePoint, Material material)\n"
         "{\n"
         "    RandomVec3 result;\n"
         "    result.value = reflect(surfacePoint.view, surfacePoint.normal);\n"
         "    result.probability = 1.0;\n"
         "    return result;\n"
         "}\n"
         "\n";

    s << "vec4 evaluateIdealReflectorSample(SurfacePoint surfacePoint, Material material, vec3 direction)\n"
         "{\n"
         "    float cos_a = max(0.0, dot(direction, surfacePoint.normal));\n"
         "    return material.specular / cos_a;\n"
         "}\n"
         "\n";

    s << "float idealReflectorSampleProbability(SurfacePoint surfacePoint, Material material, vec3 direction)\n"
         "{\n"
         "    return 0.0;\n"
         "}\n"
         "\n";

    // Ideal transmission
    s << "RandomVec3 generateIdealTransmissionSample(SurfacePoint surfacePoint, Material material)\n"
         "{\n"
         "    float cos_a = dot(surfacePoint.view, surfacePoint.normal);\n"
         "    bool enteringMaterial = (cos_a < 0.0);\n"
         "    vec3 normal = enteringMaterial ? surfacePoint.normal : -surfacePoint.normal;\n"
         "    float airRefractiveIndex = 1.0;\n"
         "    float eta = enteringMaterial ? airRefractiveIndex / material.refractiveIndex :\n"
         "                                   material.refractiveIndex / airRefractiveIndex;\n"
         "    cos_a = dot(surfacePoint.view, normal);\n"
         "\n"
              // Total internal reflection
         "    RandomVec3 result;\n"
         "    if (1.0 - eta * eta * (1.0 - cos_a * cos_a) < 0.0)\n"
         "        result.value = reflect(surfacePoint.view, normal);\n"
         "    else\n"
         "        result.value = refract(surfacePoint.view, normal, eta);\n"
         "    result.probability = 1.0;\n"
         "    return result;\n"
         "}\n"
         "\n";
    
    s << "vec4 evaluateIdealTransmissionSample(SurfacePoint surfacePoint, Material material, vec3 direction)\n"
         "{\n"
         "    float cos_a = abs(dot(direction, surfacePoint.normal));\n"
         "    return material.transparency / cos_a;\n"
         "}\n"
         "\n";

    s << "float idealTransmissionSampleProbability(SurfacePoint surfacePoint, Material material, vec3 direction)\n"
         "{\n"
         "    return 0.0;\n"
         "}\n"
         "\n";
}
