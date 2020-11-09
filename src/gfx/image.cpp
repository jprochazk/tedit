
#include "pch.h"
#include "image.hpp"

Image::Image(const std::string& uri, GLenum type, ImageOptions options)
  : handle_(-1)
  , width_(0)
  , height_(0)
  , type_(type)
{
    // load image data
    stbi_set_flip_vertically_on_load(true);
    uint8_t* data = stbi_load(uri.c_str(), &this->width_, &this->height_, &this->channels_, 0);

    assert(this->channels_ == 4);

    // upload data to gpu
    glGenTextures(1, &this->handle_);
    glBindTexture(type, this->handle_);
    glTexParameteri(type, GL_TEXTURE_WRAP_S, options.wrap_s);
    glTexParameteri(type, GL_TEXTURE_WRAP_T, options.wrap_t);
    glTexParameteri(type, GL_TEXTURE_MIN_FILTER, options.filter_min);
    glTexParameteri(type, GL_TEXTURE_MAG_FILTER, options.filter_mag);
    glTexImage2D(type, 0, GL_RGBA, this->width_, this->height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, &data[0]);
    glGenerateMipmap(type);

    // free image data
    stbi_image_free(data);
}

Image::~Image()
{
    glDeleteTextures(1, &this->handle_);
}

void
Image::attach(GLenum slot) const
{
    glActiveTexture(slot);
    glBindTexture(this->type_, this->handle_);
}

void
Image::detach() const
{
    glBindTexture(this->type_, NULL);
}

GLuint
Image::handle() const
{
    return this->handle_;
}

int
Image::width() const
{
    return this->width_;
}

int
Image::height() const
{
    return this->height_;
}

GLenum
Image::type() const
{
    return this->type_;
}