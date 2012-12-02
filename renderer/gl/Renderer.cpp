// Copyright (C) 2012 Sami Kyöstilä

#include "Renderer.h"
#include "renderer/Image.h"

#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <sstream>

#define ASSERT_GL() \
    do \
    { \
        GLenum status = glGetError(); \
        if (status) \
        { \
            std::ostringstream s; \
            s << "OpenGL error raised in " << __FILE__ << "() on line " << __LINE__ << ": " << std::setbase(16) << "0x" << status; \
            throw std::runtime_error(s.str()); \
        } \
    } while (0)

#define ASSERT_GL_EXTENSION(EXTNAME) \
    do \
    { \
        if (!(GLEW_ ## EXTNAME)) \
        { \
            std::ostringstream s; \
            s << "OpenGL extension " #EXTNAME " not supported"; \
            throw std::runtime_error(s.str()); \
        } \
    } while (0)

namespace gl
{

template <void (*F)(GLsizei, const GLuint*)>
class GLDeleter
{
public:
    typedef GLuint pointer;

    void operator()(GLuint id)
    {
        if (id)
            F(1, &id);
    }
};

class GLShaderDeleter
{
public:
    typedef GLuint pointer;

    void operator()(GLuint id)
    {
        if (id)
            glDeleteShader(id);
    }
};

class GLProgramDeleter
{
public:
    typedef GLuint pointer;

    void operator()(GLuint id)
    {
        if (id)
            glDeleteProgram(id);
    }
};

class GLFramebufferDeleter
{
public:
    typedef GLuint pointer;

    void operator()(GLuint id)
    {
        if (id)
            glDeleteFramebuffers(1, &id);
    }
};

typedef std::unique_ptr<GLuint, GLDeleter<glDeleteTextures>> Texture;
static Texture createTexture(GLenum target, int levels, GLenum internalFormat, int width, int height)
{
    GLuint id;
    glGenTextures(1, &id);
    glBindTexture(target, id);
    glTexStorage2D(target, levels, internalFormat, width, height);
    return Texture(id);
}

typedef std::unique_ptr<GLuint, GLShaderDeleter> Shader;
typedef std::unique_ptr<GLuint, GLProgramDeleter> Program;
typedef std::unique_ptr<GLuint, GLFramebufferDeleter> Framebuffer;

static bool compileShader(GLuint id, const std::string& source)
{
    const char* s[] =
    {
        source.c_str()
    };

    glShaderSource(id, 1, s, NULL);
    glCompileShader(id);

    std::string infoLog;
    GLint length;
    glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
    if (length > 0) {
        std::unique_ptr<char[]> logData(new char[length]);
        glGetShaderInfoLog(id, length, &length, logData.get());
        infoLog = logData.get();
    }

    GLint status;
    glGetShaderiv(id, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE)
    {
        std::cerr << "Shader compilation failed: " << infoLog << std::endl;
        return false;
    }
    return true;
}
    
static bool compileProgram(GLuint id, const std::string& vertSource, const std::string& fragSource)
{
    Shader vertShader(glCreateShader(GL_VERTEX_SHADER));
    Shader fragShader(glCreateShader(GL_FRAGMENT_SHADER));

    if (!compileShader(vertShader.get(), vertSource) || !compileShader(fragShader.get(), fragSource))
        return false;

    glAttachShader(id, vertShader.get());
    glAttachShader(id, fragShader.get());
    glLinkProgram(id);

    std::string infoLog;
    GLint length;
    glGetProgramiv(id, GL_INFO_LOG_LENGTH, &length);
    if (length > 0) {
        std::unique_ptr<char[]> logData(new char[length]);
        glGetShaderInfoLog(id, length, &length, logData.get());
        infoLog = logData.get();
    }

    GLint status;
    glGetProgramiv(id, GL_LINK_STATUS, &status);
    if (status != GL_TRUE)
    {
        std::cerr << "Program linking failed: " << infoLog << std::endl;
        return false;
    }
    return true;
}

#define SHADER(X) #X

static const char initVertShader[] = SHADER(
    void main()
    {
        gl_Position = vec4(0.0, 0.8, 1.0, 1.0);
    }
);

static const char initFragShader[] = SHADER(
    void main()
    {
        gl_FragColor = vec4(0.0, 0.8, 1.0, 1.0);
    }
);

Renderer::Renderer(const scene::Scene& scene)
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

    //Program initProgram(glCreateProgram());
    //compileProgram(initProgram.get(), initVertShader, initFragShader);
    //ASSERT_GL();

/* 
    GLuint init
    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, preview->m_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, preview->m_texture, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
*/
    glClearColor(1, .5, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

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
