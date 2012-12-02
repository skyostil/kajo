// Copyright (C) 2012 Sami Kyöstilä
#ifndef GL_GL_HELPERS_H
#define GL_GL_HELPERS_H

#include <GL/glew.h>
#include <memory>
#include <list>
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

#define SHADER(X) #X

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

class GLBufferDeleter
{
public:
    typedef GLuint pointer;

    void operator()(GLuint id)
    {
        if (id)
            glDeleteBuffers(1, &id);
    }
};

typedef std::unique_ptr<GLuint, GLDeleter<glDeleteTextures>> Texture;
Texture createTexture(GLenum target, int levels, GLenum internalFormat, int width, int height);

typedef std::unique_ptr<GLuint, GLShaderDeleter> Shader;
typedef std::unique_ptr<GLuint, GLProgramDeleter> Program;
typedef std::unique_ptr<GLuint, GLFramebufferDeleter> Framebuffer;

typedef std::unique_ptr<GLuint, GLBufferDeleter> Buffer;
Buffer createBuffer(GLenum target, size_t size, const void* data, GLenum usage);

bool compileShader(GLuint id, const std::string& source);

bool compileProgram(GLuint id, const std::string& vertSource, const std::string& fragSource,
                    const std::list<std::string>& attributes);

}

#endif // GL_GL_HELPERS_H
