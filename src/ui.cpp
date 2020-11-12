
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

std::vector<std::string>
Helper_GetUniquePaths(const std::vector<std::string>& paths, const std::vector<std::string>& existing)
{
    std::vector<std::string> unique;
    for (auto& file : paths) {
        auto relative_file_path = fs::relative(fs::absolute(file), fs::absolute(fs::current_path()));
        bool success = true;
        for (auto it = existing.begin(); it != existing.end(); ++it) {
            if (relative_file_path == fs::relative(fs::absolute(*it), fs::absolute(fs::current_path()))) {
                success = false;
                break;
            }
        }
        if (success)
            unique.push_back(relative_file_path.generic_string());
    }
    return unique;
}

void
Dialog_NewTileMap(Context* context)
{
    auto& state = context->state();
    auto* tilemap = state.tileMap;
}

void
Dialog_AddTileSet(Context* context)
{
    auto& state = context->state();
    auto* tilemap = state.tileMap;
    // we should not open the dialog if there is no open tileMap.
    assert(tilemap != nullptr);

    state.interactionBlocked = true;
    context->window()->openDialog(Window::Dialog::OpenFile,
        "Select Tile Set(s)",
        { "Image Files", "*.png", "*.jpg", "*.jpeg" },
        false,
        [=](bool success, std::vector<std::string>& input) {
            context->microtask([context, tilemap, success, input] {
                if (success) {
                    auto selection = Helper_GetUniquePaths(input, tilemap->tilesetPaths());
                    std::cout << "Added tilesets: " << std::endl;
                    for (auto& file : selection) {
                        if (!fs::exists(file))
                            continue;
                        auto* loaded = tile::TileSet::Load(file);
                        if (loaded) {
                            std::cout << file << std::endl;
                            tilemap->add(loaded);
                        } else // TODO: notify user that loading failed
                            continue;
                    }
                }
                context->state().interactionBlocked = false;
            });
        });
}

void
Render_TileSetWindow(Context* context, ImGuiIO& io)
{
    ImGui::SetNextWindowSize(ImVec2(320, 380));
    auto flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize |
                 ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar;
    bool wnd_open = ImGui::Begin("Tile Set", NULL, flags);
    if (!wnd_open)
        return ImGui::End();

    auto& state = context->state();
    auto* window = context->window();

    if (ImGui::BeginMenuBar()) {
        if ((ImGui::MenuItem("New", "CTRL+N") || window->shortcut(Window::Modifier::CONTROL, GLFW_KEY_N)) &&
            !ImGui::IsPopupOpen(NULL, ImGuiPopupFlags_AnyPopup)) {
            if (!state.tileMapSaved) {
                context->confirm("Would you like to save first?", [&](bool choice) {
                    if (choice) {
                        spdlog::info("Save Tile Map");
                    } else {
                        spdlog::info("Discard Tile Map");
                    }
                    spdlog::info("New Tile Map");
                });
            } else {
                spdlog::info("New Tile Map");
            }
        }
        if (ImGui::MenuItem("Save", "CTRL+S") || window->shortcut(Window::Modifier::CONTROL, GLFW_KEY_S)) {
            if (!state.tileMapSaved) {
                if (state.tileMapPath.empty() && !ImGui::IsPopupOpen(NULL, ImGuiPopupFlags_AnyPopup)) {
                    // where to save?
                    spdlog::info("Save Tile Map As");
                } else {
                    spdlog::info("Save Tile Map");
                }
            }
        }
        if ((ImGui::MenuItem("Save As")) && !ImGui::IsPopupOpen(NULL, ImGuiPopupFlags_AnyPopup)) {
            spdlog::info("Save Tile Map As");
        }
        if ((ImGui::MenuItem("Open", "CTRL+O") || window->shortcut(Window::Modifier::CONTROL, GLFW_KEY_O)) &&
            !ImGui::IsPopupOpen(NULL, ImGuiPopupFlags_AnyPopup)) {
            if (!state.tileMapSaved) {
                context->confirm("Would you like to save first?", [&](bool choice) {
                    if (choice) {
                        spdlog::info("Save Tile Map");
                    } else {
                        spdlog::info("Discard Tile Map");
                    }
                    spdlog::info("Open Tile Map");
                });
            } else {
                spdlog::info("Open Tile Map");
            }
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

        if (ImGui::SmallButton("+") && !ImGui::IsPopupOpen(NULL, ImGuiPopupFlags_AnyPopup)) {
            spdlog::info("Add tileset");
            Dialog_AddTileSet(context);
        }
    }
    { // tileset display
        std::vector<tile::TileSet*>* tilesets = nullptr;
        if (tilemap != nullptr) {
            tilesets = &tilemap->tilesets();
        }

        if (tilesets != nullptr && state.tileSetIndex < tilesets->size()) {
            // TODO: always zoom into current middle of the view instead of origin
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
            auto display_rect_flags = ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight |
                                      ImGuiButtonFlags_MouseButtonMiddle;
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

            draw_list->PushClipRect(display_p0, display_p1, true);

            // Get selected tileset image
            auto* currentTileSet = (*tilesets)[state.tileSetIndex];
            auto* currentTileSetAtlas = &currentTileSet->atlas();

            // draw tileset
            auto tileset_p0 = ImVec2(origin.x, origin.y - (currentTileSetAtlas->height() / zoom) + display_sz.y);
            auto tileset_p1 = ImVec2(origin.x + (currentTileSetAtlas->width() / zoom), origin.y + display_sz.y);
            // we flip image data on load (src/gfx/image.cpp, Image constructor),
            // but ImGUI also flips them, so we need to flip again to draw correctly.
            draw_list->AddImage(
                (void*)(currentTileSetAtlas->handle()), tileset_p0, tileset_p1, ImVec2(0, 1), ImVec2(1, 0));

            auto zoomed_tileSize = tilemap->tileSize() / zoom;
            if (is_hovered) {
                // calculate mouse pos on atlas
                auto mx = (mouse_pos_in_display.x) * zoom;
                auto my = -(mouse_pos_in_display.y - display_sz.y) * zoom;

                // check if mouse is intersecting atlas
                auto atlasW = (float)currentTileSetAtlas->width();
                auto atlasH = (float)currentTileSetAtlas->height();
                if (mx > 0 && mx < atlasW && my > 0 && my < atlasH) {
                    auto tile_column = std::floorf(mx / tilemap->tileSize());
                    auto tile_row = std::floorf(my / tilemap->tileSize());

                    // draw tile highlight
                    auto hover_p0 = ImVec2(origin.x + (zoomed_tileSize * tile_column),
                        origin.y + display_sz.y - (zoomed_tileSize * tile_row));
                    auto hover_p1 = ImVec2(hover_p0.x + zoomed_tileSize, hover_p0.y - zoomed_tileSize);
                    draw_list->AddRectFilled(hover_p0, hover_p1, IM_COL32(120, 120, 120, 100));

                    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                        // calculate tile ID
                        auto tilesPerRow = atlasW / (float)tilemap->tileSize();
                        auto tileSetId = state.tileSetIndex;
                        // X and Y are reversed because of UV flip when drawing tileset atlas (see above)
                        auto tile_id = tile_row + tile_column * tilesPerRow;

                        // set current tile
                        state.currentTile = 0;
                        tile::TileSetId(state.currentTile, state.tileSetIndex);
                        tile::TileId(state.currentTile, tile_id);
                    }
                }
            }

            // draw active tile
            if (state.tileSetIndex == tile::TileSetId(state.currentTile)) {
                auto currentTileId = tile::TileId(state.currentTile);
                auto uvPerRow = (currentTileSetAtlas->width() / tilemap->tileSize());
                auto tile_x = zoomed_tileSize * (currentTileId / uvPerRow);
                auto tile_y = zoomed_tileSize * (currentTileId % uvPerRow);
                auto active_p0 = ImVec2(origin.x + tile_x, origin.y + display_sz.y - tile_y);
                auto active_p1 = ImVec2(active_p0.x + zoomed_tileSize, active_p0.y - zoomed_tileSize);
                draw_list->AddRect(active_p0, active_p1, IM_COL32(84, 229, 255, 255), 0.0f, 15, 3.0f);
            }

            draw_list->PopClipRect();
        }
    }

    /* Render popups */
    {
        auto& currentDialog = state.currentDialog;
        if (!currentDialog.done && !ImGui::IsPopupOpen(NULL, ImGuiPopupFlags_AnyPopup)) {
            ImGui::OpenPopup("confirm_dialog");
        }
        auto popup_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize;
        if (ImGui::BeginPopupModal("confirm_dialog", NULL, popup_flags)) {
            ImGui::Text(currentDialog.text);
            ImGui::Separator();
            if (ImGui::Button("Yes")) {
                if (currentDialog.callback)
                    currentDialog.callback(true);
                currentDialog.done = true;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("No")) {
                if (currentDialog.callback)
                    currentDialog.callback(false);
                currentDialog.done = true;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        if (state.interactionBlocked && !ImGui::IsPopupOpen(NULL, ImGuiPopupFlags_AnyPopup)) {
            ImGui::OpenPopup("interaction_blocker");
        }
        auto ib_flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(ImVec2(-100, -100), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(1, 1));
        if (ImGui::BeginPopupModal("interaction_blocker", NULL, ib_flags)) {
            if (!state.interactionBlocked) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
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

    this->state_.hasMouseFocus = io.WantCaptureMouse;
    this->state_.hasKeyboardFocus = io.WantCaptureKeyboard;

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
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
Context::confirm(const char* text, std::function<void(bool)> callback)
{
    auto& currentDialog = this->state_.currentDialog;
    if (currentDialog.done)
        currentDialog = { text, callback, false };
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