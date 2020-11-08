
#include "ui.hpp"
#include "window.hpp"
#include "tile.hpp"
#define IMGUI_IMPL_OPENGL_LOADER_GL3W
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>

// WARNING: Does not work with multiple contexts

namespace ui {

void
Init_ImGUI(GLFWwindow* window)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(NULL);

    auto& io = ImGui::GetIO();

    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
}

void
Deinit_ImGUI()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void
Render_TileSetWindow(Context* context, ImGuiIO& io)
{
    bool wnd_open = ImGui::Begin("Tile Set", NULL, ImGuiWindowFlags_MenuBar);
    if (!wnd_open)
        return ImGui::End();

    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Save", "CTRL+S")) {
                context->openDialog(Dialog::SAVE);
            }
            if (ImGui::MenuItem("Open", "CTRL+O")) {
                context->openDialog(Dialog::OPEN);
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    ImGui::End();
}

Context::Context(Window* window)
  : window(window)
  , state()
{
    Init_ImGUI(this->window->getHandle());
}

Context::~Context()
{
    Deinit_ImGUI();
}

void
Context::render()
{
    ImGuiIO& io = ImGui::GetIO();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::ShowDemoWindow();

    Render_TileSetWindow(this, io);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void
Context::openDialog(Dialog dialog)
{
    switch (dialog) {
        case Dialog::SAVE: {
            spdlog::info("Dialog::SAVE");
            break;
        }
        case Dialog::OPEN: {
            spdlog::info("Dialog::OPEN");
            break;
        }
    }
}

ContextState&
Context::getState()
{
    return this->state;
}

const ContextState&
Context::getState() const
{
    return this->state;
}

}; // namespace ui