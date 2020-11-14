
#include "pch.h"
#include "mesh.hpp"

namespace gfx {

size_t
gl_sizeof(GLenum type)
{
    switch (type) {
        case GL_BYTE:
            return sizeof(uint8_t);
        case GL_FLOAT:
            return sizeof(float);
        case GL_DOUBLE:
            return sizeof(double);
        case GL_SHORT:
            return sizeof(short);
        case GL_INT:
            return sizeof(int);
        default:
            std::string msg = "Invalid type: ";
            msg += std::to_string(type);
            throw std::runtime_error{ msg };
    }
}

Mesh::Mesh(const std::vector<float>& vertices,
    const std::vector<uint32_t>& indices,
    const std::vector<Attribute>& attributes)
  : vbo_()
  , ebo_()
  , vao_()
  , count_(indices.size())
{
    glGenBuffers(1, &this->vbo_);
    glGenBuffers(1, &this->ebo_);
    glGenVertexArrays(1, &this->vao_);

    // vertex array
    glBindVertexArray(this->vao_);

    // index buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ebo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * indices.size(), indices.data(), GL_STATIC_DRAW);

    // vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, this->vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

    // calculate attribute stride
    int stride = 0;
    for (const auto& attribute : attributes) {
        stride += gl_sizeof(attribute.type) * attribute.size;
    }

    // vertex buffer descriptors
    int offset = 0;
    for (const auto& attribute : attributes) {
        glVertexAttribPointer(attribute.location, attribute.size, attribute.type, false, stride, (void*)offset);
        glEnableVertexAttribArray(attribute.location);
        offset += gl_sizeof(attribute.type) * attribute.size;
    }

    glBindVertexArray(NULL);
}

Mesh::~Mesh()
{
    glDeleteBuffers(1, &this->vbo_);
    glDeleteBuffers(1, &this->ebo_);
    glDeleteVertexArrays(1, &this->vao_);
}

void
Mesh::attach() const
{
    glBindVertexArray(this->vao_);
}

void
Mesh::draw(GLenum mode) const
{
    glDrawElements(mode, this->count_, GL_UNSIGNED_INT, NULL);
}

} // namespace gfx