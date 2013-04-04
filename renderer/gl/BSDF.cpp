#include "BSDF.h"

using namespace gl;

void BSDF::writeBSDFFunctions(std::ostringstream& s)
{
    s << "RandomVec3 generateLambertSample(SurfacePoint surfacePoint)\n"
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

    s << "vec4 evaluateLambertSample(Material material)\n"
         "{\n"
         "    return material.diffuse * M_1_PI;\n"
         "}\n"
         "\n";

    s << "float lambertSampleProbability(SurfacePoint surfacePoint, vec3 direction)\n"
         "{\n"
         "    float cos_theta = dot(direction, surfacePoint.normal);\n"
         "    return M_1_PI * cos_theta;\n"
         "}\n"
         "\n";
}
