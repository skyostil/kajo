// Copyright (C) 2012 Sami Kyöstilä

#include "Renderer.h"
#include "renderer/GLHelpers.h"
#include "renderer/Image.h"
#include "renderer/Util.h"
#include "Scene.h"

#include <glm/gtc/matrix_transform.hpp>
#include <sstream>

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
    glUniform3f(glGetUniformLocation(initProgram.get(), "imageOrigin"), p1.x, p1.y, p1.z);
    glUniform3f(glGetUniformLocation(initProgram.get(), "imageRight"), p2.x, p2.y, p2.z);
    glUniform3f(glGetUniformLocation(initProgram.get(), "imageDown"), p3.x, p3.y, p3.z);
    glUniform3f(glGetUniformLocation(initProgram.get(), "rayOrigin"), origin.x, origin.y, origin.z);
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
    ASSERT_GL();

    glBindBuffer(GL_ARRAY_BUFFER, quadBuffer.get());
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    ASSERT_GL();

    // Generate tracer program
    std::ostringstream s;
    s << "#version 120\n";
    s << SHADER(
        varying vec2 imagePosition;
        uniform sampler2D rayOrigin;
        uniform sampler2D rayDirection;

        void main()
        {
            vec3 origin = texture2D(rayOrigin, imagePosition).xyz;
            vec3 direction = texture2D(rayDirection, imagePosition).xyz;
            float minDistance = 0.0;
            float maxDistance = 1e8;
            //gl_FragColor = vec4(imagePosition, 0.0, 1.0);
            //gl_FragColor = vec4(abs(origin), 1.0);
            gl_FragColor = vec4(abs(direction), 1.0);
    );

    int i = 0;
    for (Sphere& sphere: m_scene->spheres) {
        i++;
        if (i == 2)
            break;
        s << "    {\n";
        s << "        mat4 transform = mat4(" <<
             sphere.transform.matrix[0][0] << ", " <<
             sphere.transform.matrix[0][1] << ", " <<
             sphere.transform.matrix[0][2] << ", " <<
             sphere.transform.matrix[0][3] << ", " <<
             sphere.transform.matrix[1][0] << ", " <<
             sphere.transform.matrix[1][1] << ", " <<
             sphere.transform.matrix[1][2] << ", " <<
             sphere.transform.matrix[1][3] << ", " <<
             sphere.transform.matrix[2][0] << ", " <<
             sphere.transform.matrix[2][1] << ", " <<
             sphere.transform.matrix[2][2] << ", " <<
             sphere.transform.matrix[2][3] << ", " <<
             sphere.transform.matrix[3][0] << ", " <<
             sphere.transform.matrix[3][1] << ", " <<
             sphere.transform.matrix[3][2] << ", " <<
             sphere.transform.matrix[3][3] << ");\n";
        s << "        mat4 invTransform = mat4(" <<
             sphere.transform.invMatrix[0][0] << ", " <<
             sphere.transform.invMatrix[0][1] << ", " <<
             sphere.transform.invMatrix[0][2] << ", " <<
             sphere.transform.invMatrix[0][3] << ", " <<
             sphere.transform.invMatrix[1][0] << ", " <<
             sphere.transform.invMatrix[1][1] << ", " <<
             sphere.transform.invMatrix[1][2] << ", " <<
             sphere.transform.invMatrix[1][3] << ", " <<
             sphere.transform.invMatrix[2][0] << ", " <<
             sphere.transform.invMatrix[2][1] << ", " <<
             sphere.transform.invMatrix[2][2] << ", " <<
             sphere.transform.invMatrix[2][3] << ", " <<
             sphere.transform.invMatrix[3][0] << ", " <<
             sphere.transform.invMatrix[3][1] << ", " <<
             sphere.transform.invMatrix[3][2] << ", " <<
             sphere.transform.invMatrix[3][3] << ");\n";
        s << "        float determinant = " << sphere.transform.determinant << ";\n";
        s << "        float radius2 = " << sphere.radius * sphere.radius << ";\n";
        s << SHADER(
                    vec3 localDir = mat3(invTransform) * direction;
                    vec3 localOrigin = (invTransform * vec4(origin, 1.0)).xyz;
                    float a = dot(localDir, localDir);
                    float b = 2.0 * dot(localDir, localOrigin);
                    float c = dot(localOrigin, localOrigin) - radius2;
                    float discr = b * b - 4.0 * a * c;
                    gl_FragColor.x = abs(discr);
                    if (discr > 0) {
                        float q;
                        if (b < 0)
                            q = (-b - sqrt(discr)) * .5;
                        else
                            q = (-b + sqrt(discr)) * .5;

                        float t0 = q / a;
                        float t1 = c / q;

                        if (t0 > t1) {
                            float tmp = t1;
                            t1 = t0;
                            t0 = tmp;
                        }

                        if (t1 > 0) {
                            if (t0 < 0)
                                t0 = t1;
                            vec3 normal = localOrigin + localDir * t0;
                            normal = normalize(mat3(transform) * normal);

                            t0 = determinant * t0;
                            if (t0 > minDistance && t0 < maxDistance) {
                                maxDistance = t0;
                                //gl_FragColor.z = t0;
                                //gl_FragColor = vec4(t0);
                                gl_FragColor = vec4(normal, 1.0);
                            }
                        }
                    }
                }
        );
    }

    s << SHADER(
        }
    );

    //std::cout << s.str() << '\n';

    // Intersect rays with geometry
    Program tracerProgram(glCreateProgram());
    compileProgram(tracerProgram.get(), quadVertShader, s.str(), {"position"});
    glUseProgram(tracerProgram.get());
    ASSERT_GL();

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, directionTexture.get());
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, originTexture.get());

    Sampler originSampler = createSampler();
    Sampler directionSampler = createSampler();

    glSamplerParameteri(originSampler.get(), GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glSamplerParameteri(originSampler.get(), GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glSamplerParameteri(directionSampler.get(), GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glSamplerParameteri(directionSampler.get(), GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    ASSERT_GL();

    glUniform1i(glGetUniformLocation(tracerProgram.get(), "rayOrigin"), 0);
    glBindSampler(0, originSampler.get());
    glUniform1i(glGetUniformLocation(tracerProgram.get(), "rayDirection"), 1);
    glBindSampler(1, directionSampler.get());
    ASSERT_GL();

    Texture resultTexture = createTexture(GL_TEXTURE_2D, 1, GL_RGBA8, image.width, image.height);
    Framebuffer resultFramebuffer = createFramebuffer();
    glBindFramebuffer(GL_FRAMEBUFFER, resultFramebuffer.get());
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, resultTexture.get(), 0);
    ASSERT_GL();

    glBindBuffer(GL_ARRAY_BUFFER, quadBuffer.get());
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    ASSERT_GL();
    glUseProgram(0);

    // Read back result
    //Framebuffer resultFramebuffer = createFramebuffer();
    //glBindFramebuffer(GL_FRAMEBUFFER, resultFramebuffer.get());
    //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, directionTexture.get(), 0);

    glPixelStorei(GL_PACK_ROW_LENGTH, image.width);
    glReadPixels(xOffset, yOffset, width, height, GL_BGRA, GL_UNSIGNED_BYTE, image.pixels.get());
    glPixelStorei(GL_PACK_ROW_LENGTH, 0);
    ASSERT_GL();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    ASSERT_GL();
}

}
