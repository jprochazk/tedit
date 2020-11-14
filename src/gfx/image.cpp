
#include "pch.h"
#include "image.hpp"

namespace gfx {

Image::Image(const std::string& uri, GLenum type, ImageOptions options)
  : handle_()
  , width_()
  , height_()
  , type_(type)
{
    static std::thread::id gl_thread_id = std::this_thread::get_id();
    assert(std::this_thread::get_id() == gl_thread_id);

    // load image data
    stbi_set_flip_vertically_on_load(true);
    uint8_t* data = stbi_load(uri.c_str(), &this->width_, &this->height_, &this->channels_, 0);

    auto format = GL_RGBA;
    if (this->channels_ < 4)
        format = GL_RGB;
    if (this->channels_ < 3)
        format = GL_RG;
    if (this->channels_ < 2)
        format = GL_R8;

    // upload data to gpu
    glGenTextures(1, &this->handle_);
    glBindTexture(type, this->handle_);
    glTexParameteri(type, GL_TEXTURE_WRAP_S, options.wrap_s);
    glTexParameteri(type, GL_TEXTURE_WRAP_T, options.wrap_t);
    glTexParameteri(type, GL_TEXTURE_MIN_FILTER, options.filter_min);
    glTexParameteri(type, GL_TEXTURE_MAG_FILTER, options.filter_mag);
    glTexImage2D(type, 0, format, this->width_, this->height_, 0, format, GL_UNSIGNED_BYTE, &data[0]);
    glGenerateMipmap(type);
    glBindTexture(type, NULL);

    spdlog::info("New Image, handle#{}, {}:{}", this->handle_, this->width_, this->height_);

    // free image data
    stbi_image_free(data);
}

Image::~Image()
{
    glDeleteTextures(1, &this->handle_);
}

Image::Image(Image&& other)
  : handle_(std::exchange(other.handle_, 0))
  , width_(std::exchange(other.width_, 0))
  , height_(std::exchange(other.height_, 0))
  , channels_(std::exchange(other.channels_, 0))
  , type_(other.type_)
{}
Image&
Image::operator=(Image&& other)
{
    if (this != &other) {
        this->handle_ = std::exchange(other.handle_, 0);
        this->width_ = std::exchange(other.width_, 0);
        this->height_ = std::exchange(other.height_, 0);
        this->channels_ = std::exchange(other.channels_, 0);
        this->type_ = other.type_;
    }
    return *this;
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

} // namespace gfx