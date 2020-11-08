#ifndef TEDIT_SHADER_
#define TEDIT_SHADER_

#include "pch.h"
#include "gfx/gl.h"

struct Uniform
{
    std::string name;
    GLenum type;
    int location;
    int size;
    int length;
}; // struct Uniform

class Shader final
{
public:
    Shader(const std::string& vsrc, const std::string& fsrc);
    ~Shader();

    void attach() const;
    void detach() const;

    GLuint getHandle() const;
    const Uniform& getUniform(const std::string& name) const;
    const std::unordered_map<std::string, Uniform>& getUniforms() const;

private:
    GLuint handle;
    std::unordered_map<std::string, Uniform> uniforms;
}; // class Shader

#endif // TEDIT_SHADER_
