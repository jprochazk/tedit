
#include "pch.h"
#include "mesh.hpp"

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
  : vbo()
  , ebo()
  , vao()
  , count(indices.size())
{
    glGenBuffers(1, &this->vbo);
    glGenBuffers(1, &this->ebo);
    glGenVertexArrays(1, &this->vao);

    // vertex array
    glBindVertexArray(this->vao);

    // index buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * indices.size(), indices.data(), GL_STATIC_DRAW);

    // vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

    // calculate attribute stride
    int stride = 0;
    for (const auto& attribute : attributes) {
        stride += gl_sizeof(attribute.type) * attribute.size;
    }

    spdlog::info("mesh stride: {}", stride);

    // vertex buffer descriptors
    int offset = 0;
    for (const auto& attribute : attributes) {
        glVertexAttribPointer(attribute.location, attribute.size, attribute.type, false, stride, (void*)offset);
        glEnableVertexAttribArray(attribute.location);
        spdlog::info("offset {}", offset);
        offset += gl_sizeof(attribute.type) * attribute.size;
    }

    glBindVertexArray(NULL);
}

Mesh::~Mesh()
{
    glDeleteBuffers(1, &this->vbo);
    glDeleteBuffers(1, &this->ebo);
    glDeleteVertexArrays(1, &this->vao);
}

void
Mesh::attach() const
{
    glBindVertexArray(this->vao);
}

void
Mesh::draw(GLenum mode) const
{
    glDrawElements(mode, this->count, GL_UNSIGNED_INT, NULL);
}