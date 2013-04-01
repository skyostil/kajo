// Copyright (C) 2012 Sami Kyöstilä

#include "GLHelpers.h"

GLObject::GLObject(): name(0)
{
}

GLObject::GLObject(std::nullptr_t): name(0)
{
}

GLObject::GLObject(GLuint name): name(name)
{
}

GLObject& GLObject::operator=(std::nullptr_t)
{
    name = 0;
    return *this;
}

GLObject::operator GLuint() const
{
    return name;
}

GLObject::operator bool() const
{
    return name != 0;
}

bool operator==(GLObject lhs, GLObject rhs)
{
    return lhs.name == rhs.name;

}

bool operator!=(GLObject lhs, GLObject rhs)
{
    return lhs.name != rhs.name;
}

Texture createTexture(GLenum target, int levels, GLenum internalFormat, int width, int height)
{
    GLuint id;
    glGenTextures(1, &id);
    glBindTexture(target, id);
    glTexStorage2D(target, levels, internalFormat, width, height);
    ASSERT_GL();
    return Texture(id);
}

Framebuffer createFramebuffer()
{
    GLuint id;
    glGenFramebuffers(1, &id);
    return Framebuffer(id);
}

Sampler createSampler()
{
    GLuint id;
    glGenSamplers(1, &id);
    return Sampler(id);
}

Buffer createBuffer(GLenum target, size_t size, const void* data, GLenum usage)
{
    GLuint id;
    glGenBuffers(1, &id);
    glBindBuffer(target, id);
    glBufferData(target, size, data, usage);
    return Buffer(id);
}

bool compileShader(GLuint id, const std::string& source)
{
    const char* s[] = {
        source.c_str()
    };

    glShaderSource(id, 1, s, NULL);
    glCompileShader(id);

    std::string infoLog;
    GLint length = 0;
    glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
    if (length > 0) {
        std::unique_ptr<char[]> logData(new char[length]);
        glGetShaderInfoLog(id, length, &length, logData.get());
        infoLog = logData.get();
    }

    GLint status;
    glGetShaderiv(id, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE) {
        std::cerr << "Shader compilation failed:" << std::endl;
        std::cerr << source << std::endl;
        std::cerr << infoLog << std::endl;
        return false;
    }
    return true;
}

bool compileProgram(GLuint id, const std::string& vertSource, const std::string& fragSource,
                    const std::list<std::string>& attributes)
{
    Shader vertShader(glCreateShader(GL_VERTEX_SHADER));
    Shader fragShader(glCreateShader(GL_FRAGMENT_SHADER));

    if (!compileShader(vertShader.get(), vertSource) || !compileShader(fragShader.get(), fragSource))
        return false;

    glAttachShader(id, vertShader.get());
    glAttachShader(id, fragShader.get());

    int n = 0;
    for (const std::string& attribute: attributes)
        glBindAttribLocation(id, n++, attribute.c_str());
    glLinkProgram(id);

    std::string infoLog;
    GLint length = 0;
    glGetProgramiv(id, GL_INFO_LOG_LENGTH, &length);
    if (length > 0) {
        std::unique_ptr<char[]> logData(new char[length]);
        glGetProgramInfoLog(id, length, &length, logData.get());
        infoLog = logData.get();
    }

    GLint status;
    glGetProgramiv(id, GL_LINK_STATUS, &status);
    if (status != GL_TRUE) {
        std::cerr << "Program linking failed: " << infoLog << std::endl;
        return false;
    }
    ASSERT_GL();
    return true;
}

GLint uniform(GLuint program, const char* name)
{
    GLint result = glGetUniformLocation(program, name);
    if (result == -1) {
        std::ostringstream s;
        s << "Unknown uniform: " << name << std::endl;
        throw std::runtime_error(s.str());
    }
    return result;
}
