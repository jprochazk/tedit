
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
  : handle_(0)
  , uniforms_()
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
    this->handle_ = glCreateProgram();
    glAttachShader(this->handle_, vertex);
    glAttachShader(this->handle_, fragment);
    glLinkProgram(this->handle_);
    checkProgram(this->handle_);
    // discard intermediates
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    queryUniforms(this->handle_, this->uniforms_);
}

Shader::~Shader()
{
    glDeleteProgram(this->handle_);
}

void
Shader::attach() const
{
    glUseProgram(this->handle_);
}

void
Shader::detach() const
{
    glUseProgram(NULL);
}

GLuint
Shader::handle() const
{
    return this->handle_;
}

const Uniform&
Shader::uniform(const std::string& name) const
{
    return this->uniforms_.at(name);
}

const std::unordered_map<std::string, Uniform>&
Shader::uniforms() const
{
    return this->uniforms_;
}