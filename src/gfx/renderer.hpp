#ifndef TEDIT_RENDERER_
#define TEDIT_RENDERER_

#include "pch.h"
#include "gfx/gl.h"
#include "gfx/shader.hpp"
#include "gfx/mesh.hpp"

class Window;
class Image;

struct Command
{
    Image* image;
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
    void begin(Window& window);
    /**
     * Submits a draw command.
     */
    void draw(Image* image, glm::vec4 uv = glm::vec4(0.f, 0.f, 1.f, 1.f), glm::mat4 model = glm::mat4(1));
    /**
     * Flush draw commands to the GPU.
     */
    void flush();

private:
    // textured quad shader
    Shader shader;
    // quad mesh
    Mesh mesh;
    // draw command buffer
    std::vector<Command> commands;

    // uniforms
    Uniform uProj;
    Uniform uView;
    Uniform uModel;
    Uniform uUV;
    Uniform uTexture;
}; // class Renderer

#endif // TEDIT_RENDERER_
