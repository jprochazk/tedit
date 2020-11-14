#include "pch.h"
#ifndef TEDIT_RENDERER_
#define TEDIT_RENDERER_

#include "gfx/gl.h"
#include "gfx/shader.hpp"
#include "gfx/mesh.hpp"

namespace gfx {

class Camera;
class Image;

class Renderer final
{
public:
    struct ImageCommand
    {
        const Image* image;
        glm::vec4 uv;
        glm::mat4 model;
    }; // struct ImageCommand

    struct LineCommand
    {
        glm::vec2 start;
        glm::vec2 end;
        glm::vec4 color;
        float thickness;
    }; // struct LineCommand

    Renderer();
    ~Renderer() = default;

    // TODO: this should be camera, not window
    /**
     * Submits an Image draw command.
     */
    void submit(const Image* image, glm::vec4 uv = { 0.f, 0.f, 1.f, 1.f }, glm::mat4 model = glm::mat4(1));
    /**
     * Submits a Line draw command.
     */
    void submit(glm::vec2 start, glm::vec2 end, glm::vec4 color = { 1.f, 1.f, 1.f, 1.f }, float thickness = 1.0f);
    /**
     * Render primitives from POV of `camera`.
     */
    void render(Camera& camera);

private:
    // textured quad shader
    Shader shader_;
    // quad mesh
    Mesh mesh_;
    // draw command buffers
    struct CommandBuffers
    {
        std::vector<ImageCommand> image;
        std::vector<LineCommand> line;
    } commands_;

    // uniforms
    Uniform uProj_;
    Uniform uView_;
    Uniform uModel_;
    Uniform uUV_;
    Uniform uTexture_;
}; // class Renderer

} // namespace gfx

#endif // TEDIT_RENDERER_
