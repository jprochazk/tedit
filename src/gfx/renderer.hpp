#include "pch.h"
#ifndef TEDIT_RENDERER_
#define TEDIT_RENDERER_

#include "gfx/gl.h"
#include "gfx/shader.hpp"
#include "gfx/mesh.hpp"

class Camera;
class Image;

struct Command
{
    const Image* image;
    glm::vec4 uv;
    glm::mat4 model;
}; // struct Command

class Renderer final
{
public:
    Renderer();
    ~Renderer() = default;

    // TODO: this should be camera, not window
    /**
     * Begin rendering a frame.
     *
     * Sets necessary state and clear the framebuffer.
     */
    void begin(Camera& window);
    /**
     * Submits a draw command.
     */
    void draw(const Image* image, glm::vec4 uv = glm::vec4(0.f, 0.f, 1.f, 1.f), glm::mat4 model = glm::mat4(1));
    /**
     * Flush draw commands to the GPU.
     */
    void flush();

private:
    // textured quad shader
    Shader shader_;
    // quad mesh
    Mesh mesh_;
    // draw command buffer
    std::vector<Command> commands_;

    // uniforms
    Uniform uProj_;
    Uniform uView_;
    Uniform uModel_;
    Uniform uUV_;
    Uniform uTexture_;
}; // class Renderer

#endif // TEDIT_RENDERER_
