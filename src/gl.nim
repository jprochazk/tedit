
# API:
#   type Image
#   draw(Image)
#

import nimgl/[glfw, opengl]
import stb_image/read as stbi
#import std/[strformat]
import ./log

converter toGLint(val: GLenum): GLint = int32(uint32(val))

type GLError* = object of CatchableError
#template raiseGLError(msg: untyped) = raise newException(GLError, msg)

type Image* = object
    texture: GLuint

proc `=destroy`(img: var Image) =
    glDeleteTextures(1, addr img.texture)

type ImageOptions* = object
    wrap_s*: GLenum
    wrap_t*: GLenum
    filter_min*: GLenum
    filter_mag*: GLenum

proc newImage*(src: string, options: ImageOptions): Image =
    var width, height, channels: int
    var data = stbi.load(src, width, height, channels, stbi.Default)

    assert channels == 4, "Only RGBA is supported"

    var texture: uint32
    glGenTextures(1, addr texture)
    glBindTexture(GL_TEXTURE_2D, texture)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, options.wrap_s)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, options.wrap_t)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, options.filter_min)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, options.filter_mag)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, int32(width), int32(height), 0, GL_RGBA, GL_UNSIGNED_BYTE, addr data)
    glGenerateMipmap(GL_TEXTURE_2D)

    return Image(
        texture: texture
    )

proc newImage*(src: string): Image =
    newImage(src, ImageOptions(
        wrap_s: GL_REPEAT,
        wrap_t: GL_REPEAT,
        filter_min: GL_NEAREST,
        filter_mag: GL_NEAREST
    ))
