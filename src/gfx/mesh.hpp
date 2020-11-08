#ifndef TEDIT_MESH_
#define TEDIT_MESH_

#include "pch.h"
#include "gfx/gl.h"

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

    void attach() const;
    void draw(GLenum mode) const;

private:
    GLuint vbo, ebo, vao;
    int count;
}; // class Mesh

#endif // TEDIT_MESH_
