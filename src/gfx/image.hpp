#ifndef TEDIT_IMAGE_
#define TEDIT_IMAGE_

#include "pch.h"
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

    void attach(GLenum slot) const;
    void detach() const;

    GLuint getHandle() const;
    int getWidth() const;
    int getHeight() const;
    GLenum getType() const;

private:
    GLuint handle;
    int width;
    int height;
    int channels;
    GLenum type;
}; // class Image

#endif // TEDIT_IMAGE_
