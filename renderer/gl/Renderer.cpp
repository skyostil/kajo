// Copyright (C) 2012 Sami Kyöstilä

#include "Renderer.h"
#include "renderer/Image.h"
#include "Scene.h"
#include "GLHelpers.h"

#include <glm/gtc/matrix_transform.hpp>

namespace gl
{

static const char initVertShader[] = SHADER(
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
    varying vec2 imagePosition;

    void main()
    {
        gl_FragColor = vec4(0.0, 0.8, 1.0, 1.0);
        /*
                        glm::vec4 offset = random.generate() * .5f + glm::vec4(.5f);
                        float sx = x * pixelWidth + sampleX * sampleWidth + offset.x * sampleWidth;
                        float sy = (image.height - y) * pixelHeight + sampleY * sampleHeight + offset.y * sampleHeight;
                        glm::vec3 direction = p1 + (p2 - p1) * sx + (p3 - p1) * sy - origin;
                        direction = glm::normalize(direction);*/
    }
);

Renderer::Renderer(const scene::Scene& scene):
    m_scene(new Scene(scene))
{
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
    compileProgram(initProgram.get(), initVertShader, initFragShader, {"position"});
    ASSERT_GL();
    
    const Camera& camera = m_scene->camera;
    const glm::vec4 viewport(0, 0, 1, 1);
    glm::vec3 p1 = glm::unProject(glm::vec3(0.f, 0.f, 0.f), camera.transform, camera.projection, viewport);
    glm::vec3 p2 = glm::unProject(glm::vec3(1.f, 0.f, 0.f), camera.transform, camera.projection, viewport);
    glm::vec3 p3 = glm::unProject(glm::vec3(0.f, 1.f, 0.f), camera.transform, camera.projection, viewport);
    glm::vec3 origin(glm::inverse(camera.transform) * glm::vec4(0.f, 0.f, 0.f, 1.f));

    glUseProgram(initProgram.get());
    glUniform3f(glGetUniformLocation(initProgram.get(), "imageOrigin"), p1.x, p1.y, p1.z);
    glUniform3f(glGetUniformLocation(initProgram.get(), "imageRight"), p2.x, p2.y, p2.z);
    glUniform3f(glGetUniformLocation(initProgram.get(), "imageBottom"), p3.x, p3.y, p3.z);

    glBindBuffer(GL_ARRAY_BUFFER, quadBuffer.get());
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glUseProgram(0);
/* 
    GLuint init
    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, preview->m_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, preview->m_texture, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
*/
    //glClearColor(1, .5, 0, 1);
    //glClear(GL_COLOR_BUFFER_BIT);

    glPixelStorei(GL_PACK_ROW_LENGTH, image.width);
    glReadPixels(xOffset, yOffset, width, height, GL_BGRA, GL_UNSIGNED_BYTE, image.pixels.get());
    glPixelStorei(GL_PACK_ROW_LENGTH, 0);

#if 0
    const Camera& camera = m_scene->camera;

    const glm::vec4 viewport(0, 0, 1, 1);
    glm::vec3 p1 = glm::unProject(glm::vec3(0.f, 0.f, 0.f), camera.transform, camera.projection, viewport);
    glm::vec3 p2 = glm::unProject(glm::vec3(1.f, 0.f, 0.f), camera.transform, camera.projection, viewport);
    glm::vec3 p3 = glm::unProject(glm::vec3(0.f, 1.f, 0.f), camera.transform, camera.projection, viewport);
    glm::vec3 origin(glm::inverse(camera.transform) * glm::vec4(0.f, 0.f, 0.f, 1.f));

    std::unique_ptr<glm::vec4[]> radianceMap(new glm::vec4[width * height]);

    int samples = 1;
    for (int pass = 1;; pass++)
    {
        if (m_observer && !m_observer(pass, samples, xOffset, yOffset, width, height))
            return;
    }
#endif
}

}
