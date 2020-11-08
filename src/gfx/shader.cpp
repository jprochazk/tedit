
#include "pch.h"
#include "shader.hpp"

void
checkShader(GLuint handle)
{
    int success;
    glGetShaderiv(handle, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(handle, 512, NULL, infoLog);
        spdlog::error("Shader compilation failed:\n{}", infoLog);
    };
}

void
checkProgram(GLuint handle)
{
    int success;
    glGetProgramiv(handle, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(handle, 512, NULL, infoLog);
        spdlog::error("Program linking failed:\n{}", infoLog);
    };
}

void
queryUniforms(GLuint handle, std::unordered_map<std::string, Uniform>& uniforms)
{
    glUseProgram(handle);

    int count;
    glGetProgramiv(handle, GL_ACTIVE_UNIFORMS, &count);
    for (int i = 0; i < count; ++i) {
        GLenum type;
        int length, size;
        std::array<char, 64> buffer;
        glGetActiveUniform(handle, i, 64, &length, &size, &type, buffer.data());
        if (length == 0)
            continue;

        std::string name(buffer.begin(), buffer.begin() + length);
        int location = glGetUniformLocation(handle, name.c_str());

        uniforms.emplace(name, Uniform{ name, type, location, size, length });
    }

    glUseProgram(NULL);
}

Shader::Shader(const std::string& vsrc, const std::string& fsrc)
  : handle(0)
  , uniforms()
{

    GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
    // compile vertex & fragment stages
    const char* vsrc_c = vsrc.c_str();
    const char* fsrc_c = fsrc.c_str();
    glShaderSource(vertex, 1, &vsrc_c, NULL);
    glShaderSource(fragment, 1, &fsrc_c, NULL);
    glCompileShader(vertex);
    glCompileShader(fragment);
    checkShader(vertex);
    checkShader(fragment);
    // link program
    this->handle = glCreateProgram();
    glAttachShader(this->handle, vertex);
    glAttachShader(this->handle, fragment);
    glLinkProgram(this->handle);
    checkProgram(this->handle);
    // discard intermediates
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    queryUniforms(this->handle, this->uniforms);
}

Shader::~Shader()
{
    glDeleteProgram(this->handle);
}

void
Shader::attach() const
{
    glUseProgram(this->handle);
}

void
Shader::detach() const
{
    glUseProgram(NULL);
}

GLuint
Shader::getHandle() const
{
    return this->handle;
}

const Uniform&
Shader::getUniform(const std::string& name) const
{
    return this->uniforms.at(name);
}

const std::unordered_map<std::string, Uniform>&
Shader::getUniforms() const
{
    return this->uniforms;
}