// Copyright (C) 2012 Sami Kyöstilä

#include "Renderer.h"
#include "renderer/GLHelpers.h"
#include "renderer/Image.h"
#include "renderer/Util.h"
#include "Scene.h"

#include <glm/gtc/matrix_transform.hpp>
#include <sstream>
#include <cstring>

namespace gl
{

static const char quadVertShader[] = SHADER(
    attribute vec4 position;
    varying vec2 imagePosition;

    void main()
    {
        gl_Position = position;
        imagePosition = position.xy * 0.5 + vec2(0.5);
    }
);

static const char initFragShader[] = SHADER(
    uniform vec3 imageOrigin;
    uniform vec3 imageRight;
    uniform vec3 imageDown;
    uniform vec3 rayOrigin;
    varying vec2 imagePosition;

    void main()
    {
        gl_FragData[0] = vec4(rayOrigin, 0);
        gl_FragData[1] = vec4(normalize(imageOrigin + imageRight * imagePosition.x +
                                        imageDown * imagePosition.y - rayOrigin), 0);
    }
);

Renderer::Renderer(const scene::Scene& scene):
    m_scene(new Scene(scene))
{
}

static GLint uniform(GLuint program, const char* name)
{
    GLint result = glGetUniformLocation(program, name);
    if (result == -1) {
        std::ostringstream s;
        s << "Unknown uniform: " << name << std::endl;
        throw std::runtime_error(s.str());
    }
    return result;
}

void Renderer::render(Image& image, int xOffset, int yOffset, int width, int height) const
{
    // Each ray has:
    // - origin
    // - direction
    // - weight
    // - color
    //
    // 1. Generate initial set of rays from camera
    // 2. Repeat:
    // - Intersect all rays
    // - Generate new rays based on shading
    
    ASSERT_GL();

    Texture originTexture = createTexture(GL_TEXTURE_2D, 1, GL_RGB32F, image.width, image.height);
    Texture directionTexture = createTexture(GL_TEXTURE_2D, 1, GL_RGB32F, image.width, image.height);
    Texture distanceTexture = createTexture(GL_TEXTURE_2D, 1, GL_R32F, image.width, image.height);
    ASSERT_GL();

    const float quadVertices[] = {
        -1, -1, 0, 1,
        1, -1, 0, 1,
        -1, 1, 0, 1,
        1, 1, 0, 1,
    };
    Buffer quadBuffer = createBuffer(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    ASSERT_GL();

    Program initProgram(glCreateProgram());
    compileProgram(initProgram.get(), quadVertShader, initFragShader, {"position"});
    ASSERT_GL();

    const Camera& camera = m_scene->camera;
    const glm::vec4 viewport(0, 0, 1, 1);
    glm::vec3 p1 = glm::unProject(glm::vec3(0.f, 0.f, 0.f), camera.transform, camera.projection, viewport);
    glm::vec3 p2 = glm::unProject(glm::vec3(1.f, 0.f, 0.f), camera.transform, camera.projection, viewport) - p1;
    glm::vec3 p3 = glm::unProject(glm::vec3(0.f, 1.f, 0.f), camera.transform, camera.projection, viewport) - p1;
    glm::vec3 origin(glm::inverse(camera.transform) * glm::vec4(0.f, 0.f, 0.f, 1.f));

    // Initialize origin and direction textures
    glUseProgram(initProgram.get());
    glUniform3f(uniform(initProgram.get(), "imageOrigin"), p1.x, p1.y, p1.z);
    glUniform3f(uniform(initProgram.get(), "imageRight"), p2.x, p2.y, p2.z);
    glUniform3f(uniform(initProgram.get(), "imageDown"), p3.x, p3.y, p3.z);
    glUniform3f(uniform(initProgram.get(), "rayOrigin"), origin.x, origin.y, origin.z);
    ASSERT_GL();

    Framebuffer initFramebuffer = createFramebuffer();
    glBindFramebuffer(GL_FRAMEBUFFER, initFramebuffer.get());
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, originTexture.get(), 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, directionTexture.get(), 0);
    ASSERT_GL();

    const GLenum attachments[] = {
        GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1
    };
    glDrawBuffers(2, attachments);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    ASSERT_GL();

    glBindBuffer(GL_ARRAY_BUFFER, quadBuffer.get());
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    ASSERT_GL();

    // Generate intersector program
    std::ostringstream s;
    s << "#version 120\n" // for mat3 casts
         "varying vec2 imagePosition;\n"
         "uniform sampler2D rayOrigin;\n"
         "uniform sampler2D rayDirection;\n"
         "\n";

    for (size_t i = 0; i < m_scene->planes.size(); i++) {
        std::string name = "intersectPlane" + std::to_string(i);
        m_scene->planes[i].writeIntersector(s, name);
        s << "\n";
    }

    for (size_t i = 0; i < m_scene->spheres.size(); i++) {
        std::string name = "intersectSphere" + std::to_string(i);
        m_scene->spheres[i].writeIntersector(s, name);
        s << "\n";
    }

    s << "void main()\n"
         "{\n"
         "    vec3 origin = texture2D(rayOrigin, imagePosition).xyz;\n"
         "    vec3 direction = texture2D(rayDirection, imagePosition).xyz;\n"
         "    float minDistance = 0.0;\n"
         "    float maxDistance = 1e16;\n"
         "    vec3 normal;\n"
         "\n";

    for (size_t i = 0; i < m_scene->planes.size(); i++) {
        std::string name = "intersectPlane" + std::to_string(i);
        s << "    " << name << "(origin, direction, minDistance, maxDistance, normal);\n";
    }

    for (size_t i = 0; i < m_scene->spheres.size(); i++) {
        std::string name = "intersectSphere" + std::to_string(i);
        s << "    " << name << "(origin, direction, minDistance, maxDistance, normal);\n";
    }

    s << "\n"
         "    gl_FragColor = vec4(abs(normal), 1.0);\n"
         "}\n";

    std::cout << s.str() << '\n';

    // Intersect rays with geometry
    Program tracerProgram(glCreateProgram());
    compileProgram(tracerProgram.get(), quadVertShader, s.str(), {"position"});
    glUseProgram(tracerProgram.get());
    ASSERT_GL();

    Sampler originSampler = createSampler();
    Sampler directionSampler = createSampler();

    glSamplerParameteri(originSampler.get(), GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glSamplerParameteri(originSampler.get(), GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glSamplerParameteri(directionSampler.get(), GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glSamplerParameteri(directionSampler.get(), GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    ASSERT_GL();

    Texture resultTexture = createTexture(GL_TEXTURE_2D, 1, GL_RGBA8, image.width, image.height);
    Framebuffer resultFramebuffer = createFramebuffer();
    glBindFramebuffer(GL_FRAMEBUFFER, resultFramebuffer.get());
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, resultTexture.get(), 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    ASSERT_GL();

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, directionTexture.get());
    glUniform1i(uniform(tracerProgram.get(), "rayDirection"), 1);
    glBindSampler(1, directionSampler.get());

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, originTexture.get());
    glUniform1i(uniform(tracerProgram.get(), "rayOrigin"), 0);
    glBindSampler(0, originSampler.get());
    ASSERT_GL();

    glBindBuffer(GL_ARRAY_BUFFER, quadBuffer.get());
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    ASSERT_GL();

    glUseProgram(0);
    glBindSampler(0, 0);
    glBindSampler(1, 0);

    // Read back result
    //Framebuffer resultFramebuffer = createFramebuffer();
    //glBindFramebuffer(GL_FRAMEBUFFER, resultFramebuffer.get());
    //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, originTexture.get(), 0);
    //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, directionTexture.get(), 0);

    glPixelStorei(GL_PACK_ROW_LENGTH, image.width);
    glReadPixels(xOffset, yOffset, width, height, GL_BGRA, GL_UNSIGNED_BYTE, image.pixels.get());
    glPixelStorei(GL_PACK_ROW_LENGTH, 0);
    ASSERT_GL();

    for (int y = 0; y < image.height / 2; y++) {
        for (int x = 0; x < image.width; x++) {
           std::swap(image.pixels[y * image.width + x],
                     image.pixels[(image.height - 1 - y) * image.width + x]);
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    ASSERT_GL();
}

}
