#include "pch.h"
#ifndef TEDIT_MESH_
#define TEDIT_MESH_

#include "gfx/gl.h"

namespace gfx {

struct Attribute
{
    GLuint location;
    GLuint size;
    GLenum type;
}; // struct Attribute

class Mesh final
{
public:
    Mesh(const std::vector<float>& vertices,
        const std::vector<uint32_t>& indices,
        const std::vector<Attribute>& attributes);
    ~Mesh();
    Mesh(const Mesh& other) = delete;
    Mesh& operator=(const Mesh& other) = delete;
    Mesh(Mesh&& other);
    Mesh& operator=(Mesh&& other);

    void attach() const;
    void draw(GLenum mode) const;

private:
    GLuint vbo_, ebo_, vao_;
    int count_;
}; // class Mesh

} // namespace gfx

#endif // TEDIT_MESH_
