
#include "pch.h"
#include "renderer.hpp"
#include "window.hpp"
#include "gfx/image.hpp"

// clang-format off
const char* SHADER_VSRC = 
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
const char* SHADER_FSRC = 
"#version 330 core\n"
"out vec4 FragColor;\n"
"in vec2 vUV;\n"
"uniform sampler2D uTexture;\n"
"void main() {\n"
"    FragColor = texture(uTexture, vUV);\n"
"};\n";
std::vector<float> vertices = { 
    +1.0f, +1.0f, 1.0f, 1.0f, 
    +1.0f, -1.0f, 1.0f, 0.0f,
    -1.0f, -1.0f, 0.0f, 0.0f, 
    -1.0f, +1.0f, 0.0f, 1.0f 
};
std::vector<uint32_t> indices = { 
    0, 1, 3, 
    1, 2, 3 
};
std::vector<Attribute> attributes = { 
    { 0, 2, GL_FLOAT }, 
    { 1, 2, GL_FLOAT } 
};
// clang-format on

Renderer::Renderer()
  : shader(SHADER_VSRC, SHADER_FSRC)
  , mesh(vertices, indices, attributes)
  , commands()
{
    this->uProj = shader.getUniform("uProj");
    this->uView = shader.getUniform("uView");
    this->uModel = shader.getUniform("uModel");
    this->uUV = shader.getUniform("uUV");
    this->uTexture = shader.getUniform("uTexture");
}

void
Renderer::begin(Window& window)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.35f, 0.21f, 0.35f, 1.0f);

    glClear(GL_COLOR_BUFFER_BIT);

    // TODO: get these from camera
    float hw = (float)window.getWidth() / 2.f;
    float hh = (float)window.getHeight() / 2.f;
    glm::mat4 projection = glm::ortho(hw, -hw, -hh, hh, -1.f, 1.f);
    glm::mat4 view = glm::lookAt(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 1.f), glm::vec3(0.f, 1.f, 0.f));

    this->shader.attach();
    glUniformMatrix4fv(this->uProj.location, 1, false, glm::value_ptr(projection));
    glUniformMatrix4fv(this->uView.location, 1, false, glm::value_ptr(view));
    this->mesh.attach();
}

void
Renderer::draw(Image* image, glm::vec4 uv, glm::mat4 model)
{
    this->commands.push_back(Command{ image, uv, model });
}

void
Renderer::flush()
{
    GLuint lastTexture = static_cast<GLuint>(-1);
    for (const auto& command : this->commands) {
        if (GLuint texture = command.image->getHandle(); texture != lastTexture) {
            lastTexture = texture;
            command.image->attach(GL_TEXTURE0);
            glUniform1i(this->uTexture.location, 0);
        }
        glUniform4fv(this->uUV.location, 1, glm::value_ptr(command.uv));
        glUniformMatrix4fv(this->uModel.location, 1, false, glm::value_ptr(command.model));

        this->mesh.draw(GL_TRIANGLES);
    }
    this->commands.clear();
}