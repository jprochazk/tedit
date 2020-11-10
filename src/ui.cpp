
#include "pch.h"
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
Dialog_AddTileSet(Context* context, ImGuiIO& io)
{
    auto* tilemap = context->state().tileMap;
    // we should not open the dialog if there is no open tileMap.
    assert(tilemap != nullptr);
    auto callback = [=](bool success, std::vector<std::string>& input) {
        context->microtask([=] {
            if (success) {
                std::vector<std::string> selection = input;
                // remove non-unique tileset paths
                for (auto& file : selection) {
                    file = fs::relative(fs::absolute(file), fs::absolute(fs::current_path())).generic_string();
                }
                for (auto it = selection.begin(); it != selection.end();) {
                    bool success = true;
                    for (auto& existing : tilemap->tilesetPaths()) {
                        if (*it == existing) {
                            selection.erase(it);
                            success = false;
                        } else {
                            ++it;
                        }
                    }
                }
                std::cout << "Added tilesets: " << std::endl;
                for (auto& file : selection) {
                    std::cout << file << std::endl;
                    tilemap->add(tile::TileSet::Load(file));
                }
            }
        });
    };
    context->window()->openDialog(
      Window::Dialog::OpenFile, "Select Tile Set(s)", { "Image Files", "*.png", "*.jpg", "*.jpg" }, callback);
}

void
Render_TileSetWindow(Context* context, ImGuiIO& io)
{
    ImGui::SetNextWindowSize(ImVec2(380, 640));
    bool wnd_open = ImGui::Begin("Tile Set", NULL, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoResize);
    if (!wnd_open)
        return ImGui::End();

    auto& state = context->state();

    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New", "CTRL+N")) {
                context->dialog(Dialog::NEW);
            }
            if (ImGui::MenuItem("Save", "CTRL+S")) {
                context->dialog(Dialog::SAVE);
            }
            if (ImGui::MenuItem("Open", "CTRL+O")) {
                context->dialog(Dialog::OPEN);
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    auto* tilemap = state.tileMap;

    { // tileset selection
        std::vector<std::string>* tspaths = nullptr;
        if (tilemap != nullptr) {
            tspaths = &tilemap->tilesetPaths();
        }

        const char* current_tileset_path = "Select a tile set...";
        if (tspaths != nullptr && state.tileSetIndex < tspaths->size()) {
            current_tileset_path = (*tspaths)[state.tileSetIndex].c_str();
        } else {
            current_tileset_path = "Select a tile set...";
        }

        if (ImGui::BeginCombo("Tilesets", current_tileset_path)) {
            if (tspaths != nullptr) {
                for (size_t i = 0; i < tspaths->size(); ++i) {
                    if (ImGui::Selectable((*tspaths)[i].c_str())) {
                        state.tileSetIndex = i;
                    }
                }
            }
            ImGui::EndCombo();
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("+")) {
            spdlog::info("Add tileset");
            Dialog_AddTileSet(context, io);
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("-")) {
            spdlog::info("Remove tileset");
        }
    }
    { // tileset display
        std::vector<tile::TileSet*>* tilesets = nullptr;
        if (tilemap != nullptr) {
            tilesets = &tilemap->tilesets();
        }

        if (tilesets != nullptr && state.tileSetIndex < tilesets->size()) {
            static ImVec2 offset(0.0f, 0.0f);
            static float zoom = 1.0f;

            ImVec2 display_p0 = ImGui::GetCursorScreenPos();    // ImDrawList API uses screen coordinates!
            ImVec2 display_sz = ImGui::GetContentRegionAvail(); // Resize display to what's available
            if (display_sz.x < 50.0f)
                display_sz.x = 50.0f;
            if (display_sz.y < 50.0f)
                display_sz.y = 50.0f;
            if (display_sz.y > 320.0f)
                display_sz.y = 320.0f;
            ImVec2 display_p1 = ImVec2(display_p0.x + display_sz.x, display_p0.y + display_sz.y);

            // Draw border and background color
            ImGuiIO& io = ImGui::GetIO();
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            draw_list->AddRectFilled(display_p0, display_p1, IM_COL32(50, 50, 50, 255));
            draw_list->AddRect(display_p0, display_p1, IM_COL32(255, 255, 255, 255));

            // This will catch our interactions
            auto display_rect_flags =
              ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight | ImGuiButtonFlags_MouseButtonMiddle;
            ImGui::InvisibleButton("display", display_sz, display_rect_flags);
            const bool is_hovered = ImGui::IsItemHovered();                        // Hovered
            const bool is_active = ImGui::IsItemActive();                          // Held
            const ImVec2 origin(display_p0.x + offset.x, display_p0.y + offset.y); // Lock scrolled origin
            const ImVec2 mouse_pos_in_display(io.MousePos.x - origin.x, io.MousePos.y - origin.y);

            const float mouse_threshold_for_pan = 0.0f;
            // pan
            if (is_active && ImGui::IsMouseDragging(ImGuiMouseButton_Middle, mouse_threshold_for_pan)) {
                offset.x += io.MouseDelta.x;
                offset.y += io.MouseDelta.y;
            }
            // zoom
            if (is_hovered) {
                zoom = std::clamp(zoom - io.MouseWheel / 10.0f, 0.1f, 5.0f);
            }

            // Get selected tileset image
            auto* currentTileSetAtlas = &((*tilesets)[state.tileSetIndex]->atlas());

            if (is_hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                auto tilesPerRow = (float)currentTileSetAtlas->width() / (float)tilemap->tileSize();
                // calculate mouse pos on atlas
                auto mx = (mouse_pos_in_display.x) * zoom;
                auto my = -(mouse_pos_in_display.y - display_sz.y) * zoom;
                // get hovered tile ID
                auto tileSetId = state.tileSetIndex;
                auto tile_x = mx / tilemap->tileSize();
                auto tile_y = my / tilemap->tileSize();
                auto tile_id = std::floorf(tile_y) + std::floorf(tile_x) * tilesPerRow;
                state.currentTile = 0;
                tile::TileSetId(state.currentTile, state.tileSetIndex);
                tile::TileId(state.currentTile, tile_id);
            }

            draw_list->PushClipRect(display_p0, display_p1, true);
            draw_list->AddImage((void*)(currentTileSetAtlas->handle()),
                                ImVec2(origin.x, origin.y - (currentTileSetAtlas->height() / zoom) + display_sz.y),
                                ImVec2(origin.x + (currentTileSetAtlas->width() / zoom), origin.y + display_sz.y),
                                // flip UVs on Y axis
                                ImVec2(0, 1),
                                ImVec2(1, 0));
            draw_list->PopClipRect();
        }
    }
    ImGui::Spacing();
    { // current tile display
        const Image* currentTileSetAtlas = nullptr;
        if (tilemap != nullptr) {
            auto tileSetIndex = tile::TileSetId(state.currentTile);
            auto* tilesets = &tilemap->tilesets();
            if (tileSetIndex < tilesets->size()) {
                currentTileSetAtlas = &((*tilesets)[tileSetIndex]->atlas());
            }
        }

        ImGui::Text("Current tile:");
        if (currentTileSetAtlas == nullptr) {
            ImGui::Text("None");
        } else {
            auto uv = tilemap->uv(state.currentTile);
            ImGui::Image((void*)(currentTileSetAtlas->handle()),
                         ImVec2(128, 128),
                         ImVec2(uv.x, uv.y + uv.w),
                         ImVec2(uv.x + uv.z, uv.y));
        }
    }

    ImGui::End();
}

Context::Context(Window* window)
  : window_(window)
  , state_()
  , tasks_()
{
    Init_ImGUI(this->window_->handle());
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
Context::dialog(Dialog dialog)
{
    switch (dialog) {
        case Dialog::NEW: {
            spdlog::info("Dialog::NEW");
            break;
        }
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

Window*
Context::window()
{
    return this->window_;
}
const Window*
Context::window() const
{
    return this->window_;
}

ContextState&
Context::state()
{
    return this->state_;
}

const ContextState&
Context::state() const
{
    return this->state_;
}

void
Context::microtask(Task&& task)
{
    this->tasks_.enqueue(std::move(task));
}

void
Context::poll()
{
    this->tasks_.poll();
}

}; // namespace ui