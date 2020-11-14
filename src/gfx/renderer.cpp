
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
"};\n";
const char* QUAD_SHADER_FSRC = 
"#version 330 core\n"
"out vec4 FragColor;\n"
"in vec2 vUV;\n"
"uniform sampler2D uTexture;\n"
"void main() {\n"
"    FragColor = texture(uTexture, vUV);\n"
"};\n";
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
"precision mediump float;\n"
"uniform mat4 uView;\n"
"uniform mat4 uProjection;\n"
"layout(location = 0) in vec2 aPosition;\n"
"layout(location = 1) in vec4 aColor;\n"
"out vec4 vColor;\n"
"void main() {\n"
"    vColor = aColor;\n"
"    gl_Position = uProjection * uView * vec4(aPosition, 0.0, 1.0);\n"
"}\n";
const char* LINE_SHADER_FSRC =
"#version 300 es"
"precision mediump float;"
"in vec4 vColor;"
"out vec4 oFragColor;"
"void main() {"
"    oFragColor = vColor;"
"}";
// clang-format on

Renderer::Renderer()
  : shader_(QUAD_SHADER_VSRC, QUAD_SHADER_FSRC)
  , mesh_(quad_vertices, quad_indices, quad_attributes)
  , commands_()
{
    this->uProj_ = this->shader_.uniform("uProj");
    this->uView_ = this->shader_.uniform("uView");
    this->uModel_ = this->shader_.uniform("uModel");
    this->uUV_ = this->shader_.uniform("uUV");
    this->uTexture_ = this->shader_.uniform("uTexture");
}

void
Renderer::submit(const Image* image, glm::vec4 uv, glm::mat4 model)
{
    this->commands_.image.push_back(ImageCommand{ image, uv, model });
}

void
Renderer::submit(glm::vec2 start, glm::vec2 end, glm::vec4 color, float thickness)
{
    (void)start;
    (void)end;
    (void)color;
    (void)thickness;
    // this->commands_.line.push_back(LineCommand{ start, end, color, thickness });
}

void
Renderer::render(Camera& camera)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(149.f / 255.f, 177.f / 255.f, 194.f / 255.f, 1.0f);

    glClear(GL_COLOR_BUFFER_BIT);

    // draw images
    {
        auto& projection = camera.projection();
        auto& view = camera.view();

        this->shader_.attach();
        glUniformMatrix4fv(this->uProj_.location, 1, false, glm::value_ptr(projection));
        glUniformMatrix4fv(this->uView_.location, 1, false, glm::value_ptr(view));
        this->mesh_.attach();

        GLuint lastTexture = static_cast<GLuint>(-1);
        for (const auto& command : this->commands_.image) {
            if (GLuint texture = command.image->handle(); texture != lastTexture) {
                lastTexture = texture;
                command.image->attach(GL_TEXTURE0);
                glUniform1i(this->uTexture_.location, 0);
            }
            glUniform4fv(this->uUV_.location, 1, glm::value_ptr(command.uv));
            glUniformMatrix4fv(this->uModel_.location, 1, false, glm::value_ptr(command.model));

            this->mesh_.draw(GL_TRIANGLES);
        }
    }

    this->commands_.image.clear();
    this->commands_.line.clear();
}

} // namespace gfx