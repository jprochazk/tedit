
#include "pch.h"
#include "renderer.hpp"
#include "window.hpp"
#include "gfx/image.hpp"
#include "gfx/camera.hpp"

namespace gfx {

// clang-format off
const char* QUAD_SHADER_VSRC = 
"#version 330 core\n"
"layout (location = 0) in vec2 aPos;\n"
"layout (location = 1) in vec2 aUV;\n"
"uniform mat4 uProj;\n"
"uniform mat4 uView;\n"
"uniform mat4 uModel;\n"
"uniform vec4 uUV;\n"
"out vec2 vUV;\n"
"void main() {\n"
"    vUV = aUV * uUV.zw + uUV.xy;\n"
"    gl_Position = uProj * uView * uModel * vec4(aPos, 0.0, 1.0);\n"
"}";
const char* QUAD_SHADER_FSRC = 
"#version 330 core\n"
"out vec4 FragColor;\n"
"in vec2 vUV;\n"
"uniform sampler2D uTexture;\n"
"void main() {\n"
"    FragColor = texture(uTexture, vUV);\n"
"}";
std::vector<float> quad_vertices = { 
    +1.0f, +1.0f, 1.0f, 1.0f, 
    +1.0f, -1.0f, 1.0f, 0.0f,
    -1.0f, -1.0f, 0.0f, 0.0f, 
    -1.0f, +1.0f, 0.0f, 1.0f 
};
std::vector<uint32_t> quad_indices = { 
    0, 1, 3, 
    1, 2, 3 
};
std::vector<Attribute> quad_attributes = { 
    { 0, 2, GL_FLOAT }, 
    { 1, 2, GL_FLOAT } 
};

const char* LINE_SHADER_VSRC =
"#version 330 core\n"
"uniform mat4 uProj;\n"
"uniform mat4 uView;\n"
"layout(location = 0) in vec2 aPosition;\n"
"void main() {\n"
"    gl_Position = uProj * uView * vec4(aPosition, 0.0, 1.0);\n"
"}";
const char* LINE_SHADER_FSRC =
"#version 330 core\n"
"uniform vec4 uColor;\n"
"out vec4 oFragColor;\n"
"void main() {\n"
"    oFragColor = uColor;\n"
"}";
// clang-format on

Renderer::Renderer()
  : shaders_({ std::move(Shader(QUAD_SHADER_VSRC, QUAD_SHADER_FSRC)), Shader(LINE_SHADER_VSRC, LINE_SHADER_FSRC) })
  , meshes_({ std::move(Mesh(quad_vertices, quad_indices, quad_attributes)), { std::move(Batch(2 << 15)), 0 } })
  , commands_()
  , uniforms_()
  , lineColor_(0, 0, 0, 1)
{
    // gather quad shader uniforms
    this->uniforms_.quad.uProj = this->shaders_.quad.uniform("uProj");
    this->uniforms_.quad.uView = this->shaders_.quad.uniform("uView");
    this->uniforms_.quad.uModel = this->shaders_.quad.uniform("uModel");
    this->uniforms_.quad.uUV = this->shaders_.quad.uniform("uUV");
    this->uniforms_.quad.uTexture = this->shaders_.quad.uniform("uTexture");

    // gather line shader uniforms
    this->uniforms_.line.uProj = this->shaders_.line.uniform("uProj");
    this->uniforms_.line.uView = this->shaders_.line.uniform("uView");
    this->uniforms_.line.uColor = this->shaders_.line.uniform("uColor");

    glGenVertexArrays(1, &this->meshes_.line.vao);
    glBindVertexArray(this->meshes_.line.vao);
    glBindBuffer(GL_ARRAY_BUFFER, this->meshes_.line.batch.handle());
    glVertexAttribPointer(0, 2, GL_FLOAT, false, sizeof(float) * 2, NULL);
    glEnableVertexAttribArray(0);
    glBindVertexArray(NULL);
}

void
Renderer::submit(const Image* image, glm::vec4 uv, glm::mat4 model)
{
    this->commands_.image.push_back(ImageCommand{ image, uv, model });
}

void
Renderer::submit(glm::vec2 start, glm::vec2 end, float thickness)
{
    auto angle = atan2f(start.y - end.y, start.x - end.x);
    float x_pad = sinf(angle) * (thickness / 2.f);
    float y_pad = cosf(angle) * (thickness / 2.f);
    glm::vec2 bottom_left = { start.x - x_pad, start.y + y_pad };
    glm::vec2 bottom_right = { start.x + x_pad, start.y - y_pad };
    glm::vec2 top_left = { end.x - x_pad, end.y + y_pad };
    glm::vec2 top_right = { end.x + x_pad, end.y - y_pad };

    float vertices[2 * 6] = {
        top_left.x,
        top_left.y,

        bottom_left.x,
        bottom_left.y,

        bottom_right.x,
        bottom_right.y,

        bottom_right.x,
        bottom_right.y,

        top_right.x,
        top_right.y,

        top_left.x,
        top_left.y,
    };

    auto& batch = this->meshes_.line.batch;
    // TODO: overflow check + split batch
    batch.insert(&vertices[0], sizeof(vertices));
    this->commands_.lines++;
}

void
Renderer::render(Camera& camera)
{
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(149.f / 255.f, 177.f / 255.f, 194.f / 255.f, 1.0f);

    glClear(GL_COLOR_BUFFER_BIT);

    auto& projection = camera.projection();
    auto& view = camera.view();
    // render images
    {
        this->shaders_.quad.attach();
        glUniformMatrix4fv(this->uniforms_.quad.uProj.location, 1, false, glm::value_ptr(projection));
        glUniformMatrix4fv(this->uniforms_.quad.uView.location, 1, false, glm::value_ptr(view));
        this->meshes_.quad.attach();

        GLuint lastTexture = static_cast<GLuint>(-1);
        for (const auto& command : this->commands_.image) {
            if (GLuint texture = command.image->handle(); texture != lastTexture) {
                lastTexture = texture;
                command.image->attach(GL_TEXTURE0);
                glUniform1i(this->uniforms_.quad.uTexture.location, 0);
            }
            glUniform4fv(this->uniforms_.quad.uUV.location, 1, glm::value_ptr(command.uv));
            glUniformMatrix4fv(this->uniforms_.quad.uModel.location, 1, false, glm::value_ptr(command.model));

            this->meshes_.quad.draw(GL_TRIANGLES);
        }
    }

    // render lines
    {
        this->shaders_.line.attach();
        glUniformMatrix4fv(this->uniforms_.line.uProj.location, 1, false, glm::value_ptr(projection));
        glUniformMatrix4fv(this->uniforms_.line.uView.location, 1, false, glm::value_ptr(view));
        glUniform4fv(this->uniforms_.line.uColor.location, 1, glm::value_ptr(this->lineColor_));

        this->meshes_.line.batch.flush();
        glBindVertexArray(this->meshes_.line.vao);
        glDrawArrays(GL_TRIANGLES, 0, this->commands_.lines * 6);
    }

    this->commands_.image.clear();
}

void
Renderer::setLineColor(glm::vec4 color)
{
    this->lineColor_ = color;
}

} // namespace gfx