
#include "image.hpp"

Image::Image(const std::string& uri, GLenum type, ImageOptions options)
  : uri(uri)
  , handle(-1)
  , width(0)
  , height(0)
  , type(type)
{
    // load image data
    stbi_set_flip_vertically_on_load(true);
    uint8_t* data = stbi_load(uri.c_str(), &this->width, &this->height, &this->channels, 0);

    assert(this->channels == 4);

    // upload data to gpu
    glGenTextures(1, &this->handle);
    glBindTexture(type, this->handle);
    glTexParameteri(type, GL_TEXTURE_WRAP_S, options.wrap_s);
    glTexParameteri(type, GL_TEXTURE_WRAP_T, options.wrap_t);
    glTexParameteri(type, GL_TEXTURE_MIN_FILTER, options.filter_min);
    glTexParameteri(type, GL_TEXTURE_MAG_FILTER, options.filter_mag);
    glTexImage2D(type, 0, GL_RGBA, this->width, this->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &data[0]);
    glGenerateMipmap(type);

    // free image data
    stbi_image_free(data);
}

Image::~Image()
{
    glDeleteTextures(1, &this->handle);
}

void
Image::attach(GLenum slot) const
{
    glActiveTexture(slot);
    glBindTexture(this->type, this->handle);
}

void
Image::detach() const
{
    glBindTexture(this->type, NULL);
}

std::string
Image::getURI() const
{
    return this->uri;
}
GLuint
Image::getHandle() const
{
    return this->handle;
}
int
Image::getWidth() const
{
    return this->width;
}
int
Image::getHeight() const
{
    return this->height;
}
GLenum
Image::getType() const
{
    return this->type;
}