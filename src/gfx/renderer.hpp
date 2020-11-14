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
    struct TexturedQuadCommand
    {
        const Image* image;
        glm::vec4 uv;
        glm::mat4 model;
    }; // struct ImageCommand

    struct ColoredQuadCommand
    {
        glm::vec4 color;
        glm::mat4 model;
    }; // struct ColoredQuadCommand

    Renderer();
    ~Renderer() = default;

    // TODO: this should be camera, not window
    /**
     * Submits a textured quad draw command.
     */
    void submit(const Image* image, glm::vec4 uv = { 0.0f, 0.0f, 1.0f, 1.0f }, glm::mat4 model = glm::mat4(1));
    /**
     * Submits a colored quad draw command.
     */
    void submit(glm::vec4 color, glm::mat4 model = glm::mat4(1));
    /**
     * Submits a line draw command. Start/end coordinates have to be in world space.
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
        Shader quad_tex;
        Shader quad_col;
        Shader line;
    } shaders_;

    struct Meshes
    {
        Mesh quad_tex;
        Mesh quad_col;
        struct
        {
            Batch batch;
            GLuint vao;
        } line;
    } meshes_;

    // draw command buffers
    struct CommandBuffers
    {
        std::vector<TexturedQuadCommand> quad_tex;
        std::vector<ColoredQuadCommand> quad_col;
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
        } quad_tex;
        struct
        {
            Uniform uProj;
            Uniform uView;
            Uniform uModel;
            Uniform uColor;
        } quad_col;
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
