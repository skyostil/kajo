// Copyright (C) 2012 Sami Kyöstilä

#include "Renderer.h"
#include "renderer/Image.h"
#include "renderer/Util.h"

#include <glm/gtc/matrix_transform.hpp>
#include <sstream>
#include <cstring>

namespace gl
{

Renderer::Renderer(const scene::Scene& scene, Image* image):
    m_image(image),
    m_scene(new Scene(scene)),
    m_random(new Random()),
    m_raytracer(new Raytracer(m_scene.get())),
    m_shader(new SurfaceShader(m_scene.get(), m_raytracer.get(), m_random.get())),
    m_originTexture(createTexture(GL_TEXTURE_2D, 1, GL_RGB32F, image->width, image->height)),
    m_directionTexture(createTexture(GL_TEXTURE_2D, 1, GL_RGB32F, image->width, image->height)),
    m_distanceNormalTexture(createTexture(GL_TEXTURE_2D, 1, GL_RGBA32F, image->width, image->height)),
    m_radianceTexture(createTexture(GL_TEXTURE_2D, 1, GL_RGBA32F, image->width, image->height)),
    m_weightTexture(createTexture(GL_TEXTURE_2D, 1, GL_RGBA32F, image->width, image->height)),
    m_newOriginTexture(createTexture(GL_TEXTURE_2D, 1, GL_RGB32F, image->width, image->height)),
    m_newDirectionTexture(createTexture(GL_TEXTURE_2D, 1, GL_RGB32F, image->width, image->height)),
    m_newRadianceTexture(createTexture(GL_TEXTURE_2D, 1, GL_RGBA32F, image->width, image->height)),
    m_newWeightTexture(createTexture(GL_TEXTURE_2D, 1, GL_RGBA32F, image->width, image->height)),
    m_tracerProgram(glCreateProgram()),
    m_shaderProgram(glCreateProgram()),
    m_originSampler(createSampler()),
    m_directionSampler(createSampler()),
    m_distanceNormalSampler(createSampler()),
    m_radianceSampler(createSampler()),
    m_weightSampler(createSampler()),
    m_distanceNormalFramebuffer(createFramebuffer()),
    m_nextIterationFramebuffer(createFramebuffer()),
    m_radianceFramebuffer(createFramebuffer()),
    m_radianceMap(new glm::vec4[image->width * image->height])
{
    ASSERT_GL();

    const float quadVertices[] = {
        -1, -1, 0, 1,
        1, -1, 0, 1,
        -1, 1, 0, 1,
        1, 1, 0, 1,
    };
    m_quadBuffer = createBuffer(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    ASSERT_GL();

    const char quadVertShader[] =
        "attribute vec4 position;\n"
        "varying vec2 imagePosition;\n"
        "\n"
        "void main()\n"
        "{\n"
        "    gl_Position = position;\n"
        "    imagePosition = position.xy * 0.5 + vec2(0.5);\n"
        "}\n";

    // Write initialization program
    std::ostringstream s;
    m_random->writeRandomNumberGenerator(s);
    m_raytracer->writeRayGenerator(s);

    s << "varying vec2 imagePosition;\n"
         "\n"
         "void main()\n"
         "{\n"
         "    vec3 origin, direction;\n"
         "    generateRay(imagePosition, origin, direction);\n"
         "    gl_FragData[0] = vec4(origin, 0.0);\n"
         "    gl_FragData[1] = vec4(direction, 0.0);\n"
         "    gl_FragData[2] = vec4(0.0);\n"
         "    gl_FragData[3] = vec4(1.0);\n"
         "}\n";

    Program initProgram(glCreateProgram());
    compileProgram(initProgram.get(), quadVertShader, s.str(), {"position"});
    ASSERT_GL();

    glUseProgram(initProgram.get());
    m_raytracer->setRayGeneratorUniforms(initProgram.get());
    ASSERT_GL();

    Framebuffer initFramebuffer = createFramebuffer();
    glBindFramebuffer(GL_FRAMEBUFFER, initFramebuffer.get());
    glViewport(0, 0, m_image->width, m_image->height);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_originTexture.get(), 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_directionTexture.get(), 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_radianceTexture.get(), 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, m_weightTexture.get(), 0);
    ASSERT_GL();

    const GLenum initAttachments[] = {
        GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2,
        GL_COLOR_ATTACHMENT3
    };
    glDrawBuffers(4, initAttachments);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    ASSERT_GL();

    drawQuad();
    glUseProgram(0);

    // Write intersector program
    s.str("");
    s << "#version 120\n" // for mat3 casts
         "varying vec2 imagePosition;\n"
         "uniform sampler2D rayOriginSampler;\n"
         "uniform sampler2D rayDirectionSampler;\n"
         "\n";

    m_raytracer->writeRayIntersector(s);

    s << "void main()\n"
         "{\n"
         "    vec3 origin = texture2D(rayOriginSampler, imagePosition).xyz;\n"
         "    vec3 direction = texture2D(rayDirectionSampler, imagePosition).xyz;\n"
         "    float distance = -1.0;\n"
         "    vec3 normal = vec3(0.0);\n"
         "    float objectIndex = 0.0;\n"
         "\n"
         "    intersectRay(origin, direction, distance, normal, objectIndex);\n"
         "\n"
         "    gl_FragData[0] = vec4(normal * (objectIndex + 1.0), distance);\n"
         "}\n";

    //std::cout << s.str() << '\n';
    compileProgram(m_tracerProgram.get(), quadVertShader, s.str(), {"position"});
    ASSERT_GL();

    // Write shader program
    s.str("");
    s << "#version 120\n"; // for first class arrays
    m_raytracer->writeRayIntersector(s);
    m_shader->writeSurfaceShader(s);

    s << "varying vec2 imagePosition;\n"
         "uniform sampler2D rayOriginSampler;\n"
         "uniform sampler2D rayDirectionSampler;\n"
         "uniform sampler2D distanceNormalSampler;\n"
         "uniform sampler2D radianceSampler;\n"
         "uniform sampler2D weightSampler;\n"
         "\n"
         "void main()\n"
         "{\n"
         "    vec3 origin = texture2D(rayOriginSampler, imagePosition).xyz;\n"
         "    vec3 direction = texture2D(rayDirectionSampler, imagePosition).xyz;\n"
         "    vec4 distanceNormal = texture2D(distanceNormalSampler, imagePosition);\n"
         "    vec4 radiance = texture2D(radianceSampler, imagePosition);\n"
         "    vec4 weight = texture2D(weightSampler, imagePosition);\n"
         "\n"
         "    vec3 normal = distanceNormal.xyz;\n"
         "    float distance = distanceNormal.w;\n"
         "    float objectIndex = length(normal);\n"
         "    normal = normal / objectIndex;\n"
         "    objectIndex = objectIndex - 1.0 + 0.5;\n"
         "\n"
         "    shadeSurfacePoint(origin, direction, distance, normal, int(objectIndex),\n"
         "                      imagePosition, radiance, weight);\n"
         "\n"
         "    gl_FragData[0] = vec4(origin, 0.0);\n"
         "    gl_FragData[1] = vec4(direction, 0.0);\n"
         "    gl_FragData[2] = radiance;\n"
         "    gl_FragData[3] = weight;\n"
         "}\n";

    std::cout << s.str();

    compileProgram(m_shaderProgram.get(), quadVertShader, s.str(), {"position"});
    ASSERT_GL();

    glUseProgram(m_shaderProgram.get());
    m_shader->setSurfaceShaderUniforms(m_shaderProgram.get());
    glUseProgram(0);
    ASSERT_GL();

    // Prepare samplers
    glSamplerParameteri(m_originSampler.get(), GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glSamplerParameteri(m_originSampler.get(), GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glSamplerParameteri(m_directionSampler.get(), GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glSamplerParameteri(m_directionSampler.get(), GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glSamplerParameteri(m_distanceNormalSampler.get(), GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glSamplerParameteri(m_distanceNormalSampler.get(), GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glSamplerParameteri(m_radianceSampler.get(), GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glSamplerParameteri(m_radianceSampler.get(), GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glSamplerParameteri(m_weightSampler.get(), GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glSamplerParameteri(m_weightSampler.get(), GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    ASSERT_GL();
}

void Renderer::drawQuad()
{
    glBindBuffer(GL_ARRAY_BUFFER, m_quadBuffer.get());
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    ASSERT_GL();
}

void Renderer::render()
{
    // Each ray has:
    // - origin
    // - direction
    // - color
    // - weight
    //
    // 1. Generate initial set of rays from camera
    // 2. Repeat:
    // - Intersect all rays. Produces distance, normal, object index. Object
    //   index is encoded into the length of the normal.
    // - Generate new rays based on shading. Produces origin, direction,
    //   radiance, weight.

    for (int pass = 0; pass < 8; pass++) {
        ASSERT_GL();

        // Intersect rays with scene geometry
        glUseProgram(m_tracerProgram.get());
        glBindFramebuffer(GL_FRAMEBUFFER, m_distanceNormalFramebuffer.get());
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_distanceNormalTexture.get(), 0);
        glViewport(0, 0, m_image->width, m_image->height);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m_directionTexture.get());
        glUniform1i(uniform(m_tracerProgram.get(), "rayDirectionSampler"), 1);
        glBindSampler(1, m_directionSampler.get());

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_originTexture.get());
        glUniform1i(uniform(m_tracerProgram.get(), "rayOriginSampler"), 0);
        glBindSampler(0, m_originSampler.get());
        ASSERT_GL();

        drawQuad();

        glUseProgram(0);
        glBindSampler(0, 0);
        glBindSampler(1, 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
        ASSERT_GL();

        // Shade and generate new rays
        glUseProgram(m_shaderProgram.get());
        m_shader->setSurfaceShaderUniforms(m_shaderProgram.get());
        glBindFramebuffer(GL_FRAMEBUFFER, m_nextIterationFramebuffer.get());
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_newOriginTexture.get(), 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_newDirectionTexture.get(), 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_newRadianceTexture.get(), 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, m_newWeightTexture.get(), 0);

        const GLenum shaderAttachments[] = {
            GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2,
            GL_COLOR_ATTACHMENT3
        };
        glDrawBuffers(4, shaderAttachments);
        ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
        ASSERT_GL();

        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, m_weightTexture.get());
        glUniform1i(uniform(m_shaderProgram.get(), "weightSampler"), 4);
        glBindSampler(4, m_weightSampler.get());

        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, m_radianceTexture.get());
        glUniform1i(uniform(m_shaderProgram.get(), "radianceSampler"), 3);
        glBindSampler(3, m_radianceSampler.get());

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, m_distanceNormalTexture.get());
        glUniform1i(uniform(m_shaderProgram.get(), "distanceNormalSampler"), 2);
        glBindSampler(2, m_distanceNormalSampler.get());

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m_directionTexture.get());
        glUniform1i(uniform(m_shaderProgram.get(), "rayDirectionSampler"), 1);
        glBindSampler(1, m_directionSampler.get());

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_originTexture.get());
        glUniform1i(uniform(m_shaderProgram.get(), "rayOriginSampler"), 0);
        glBindSampler(0, m_originSampler.get());
        ASSERT_GL();

        drawQuad();

        glUseProgram(0);
        glBindSampler(0, 0);
        glBindSampler(1, 0);
        glBindSampler(2, 0);
        glBindSampler(3, 0);
        glBindSampler(4, 0);

        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
        ASSERT_GL();

        std::swap(m_originTexture, m_newOriginTexture);
        std::swap(m_directionTexture, m_newDirectionTexture);
        std::swap(m_radianceTexture, m_newRadianceTexture);
        std::swap(m_weightTexture, m_newWeightTexture);
    }

    // Read back result
    glBindFramebuffer(GL_FRAMEBUFFER, m_radianceFramebuffer.get());
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_radianceTexture.get(), 0);

    glPixelStorei(GL_PACK_ROW_LENGTH, m_image->width);
    //glReadPixels(0, 0, m_image->width, m_image->height, GL_BGRA, GL_UNSIGNED_BYTE, m_image->pixels.get());
    glReadPixels(0, 0, m_image->width, m_image->height, GL_RGBA, GL_FLOAT, m_radianceMap.get());
    glPixelStorei(GL_PACK_ROW_LENGTH, 0);
    ASSERT_GL();

    for (int y = 0; y < m_image->height; y++) {
        for (int x = 0; x < m_image->width; x++) {
            glm::vec4 radiance = m_radianceMap[y * m_image->width + x];
            if (!radiance.w)
                continue;
            radiance.r /= radiance.w;
            radiance.g /= radiance.w;
            radiance.b /= radiance.w;
            radiance = glm::clamp(radiance, glm::vec4(0), glm::vec4(1));
            radiance.a = 1;
            uint32_t pixel = Image::colorToRGBA8(radiance);
            m_image->pixels[(m_image->height - 1 - y) * m_image->width + x] = pixel;
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    ASSERT_GL();
}

}
