#include "pch.h"
#ifndef TEDIT_SHADER_
#    define TEDIT_SHADER_

#    include "gfx/gl.h"

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

    GLuint handle() const;
    const Uniform& uniform(const std::string& name) const;
    const std::unordered_map<std::string, Uniform>& uniforms() const;

private:
    GLuint handle_;
    std::unordered_map<std::string, Uniform> uniforms_;
}; // class Shader

#endif // TEDIT_SHADER_
