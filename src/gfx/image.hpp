#include "pch.h"
#ifndef TEDIT_IMAGE_
#define TEDIT_IMAGE_

#include "gfx/gl.h"
#include <stb_image.h>

struct ImageOptions
{
    GLenum wrap_s;
    GLenum wrap_t;
    GLenum filter_min;
    GLenum filter_mag;
}; // struct ImageOptions

class Image final
{
public:
    Image(const std::string& uri, GLenum type, ImageOptions options = { GL_REPEAT, GL_REPEAT, GL_NEAREST, GL_NEAREST });
    ~Image();

    Image(const Image& other) = delete;
    Image& operator=(const Image& other) = delete;
    Image(Image&& other) = default;
    Image& operator=(Image&& other) = default;

    void attach(GLenum slot) const;
    void detach() const;

    GLuint handle() const;
    int width() const;
    int height() const;
    GLenum type() const;

private:
    GLuint handle_;
    int width_;
    int height_;
    int channels_;
    GLenum type_;
}; // class Image

#endif // TEDIT_IMAGE_
