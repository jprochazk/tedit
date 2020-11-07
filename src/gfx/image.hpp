#ifndef TEDIT_IMAGE_
#define TEDIT_IMAGE_

#include "gfx/gl.h"
#include <stb_image.h>
#include <cassert>
#include <string>

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
    Image(const std::string& uri, GLenum type, const ImageOptions& options);
    ~Image();

    void attach(GLenum slot) const;
    void detach() const;

private:
    std::string uri;
    GLuint handle;
    int width;
    int height;
    GLenum type;
}; // class Image

#endif // TEDIT_IMAGE_
