#include "pch.h"
#ifndef TEDIT_RENDERER_
#define TEDIT_RENDERER_

#include "gfx/gl.h"
#include "gfx/shader.hpp"
#include "gfx/mesh.hpp"
#include "gfx/batch.hpp"

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

    Renderer();
    ~Renderer() = default;

    // TODO: this should be camera, not window
    /**
     * Submits an Image draw command.
     */
    void submit(const Image* image, glm::vec4 uv = { 0.f, 0.f, 1.f, 1.f }, glm::mat4 model = glm::mat4(1));
    /**
     * Submits a Line draw command. Coordinates are in world space.
     */
    void submit(glm::vec2 start, glm::vec2 end, float thickness = 1.0f);
    /**
     * Render primitives from POV of `camera`.
     */
    void render(Camera& camera);

    void setLineColor(glm::vec4 color);

private:
    struct Shaders
    {
        Shader quad;
        Shader line;
    } shaders_;

    struct Meshes
    {
        Mesh quad;
        struct
        {
            Batch batch;
            GLuint vao;
        } line;
    } meshes_;

    // draw command buffers
    struct CommandBuffers
    {
        std::vector<ImageCommand> image;
        size_t lines;
    } commands_;

    // uniforms
    struct Uniforms
    {
        struct
        {
            Uniform uProj;
            Uniform uView;
            Uniform uModel;
            Uniform uUV;
            Uniform uTexture;
        } quad;
        struct
        {
            Uniform uProj;
            Uniform uView;
            Uniform uColor;
        } line;
    } uniforms_;

    glm::vec4 lineColor_;
}; // class Renderer

} // namespace gfx

#endif // TEDIT_RENDERER_
