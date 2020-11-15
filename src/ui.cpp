
#include "pch.h"
#include "ui.hpp"
#include "window.hpp"
#include "tile.hpp"
#include "misc/imgui/backends/imgui_impl_glfw.h"
#include "misc/imgui/backends/imgui_impl_opengl3.h"

namespace ImGui {
// Taken from imgui/misc/cpp/imgui_stdlib.cpp
// This is a InputText widget which accepts a std::string* instead of char*
// It automatically resizes the string and writes input to it.

namespace detail {

struct InputStringCallback_UserData
{
    std::string* Str;
    ImGuiInputTextCallback ChainCallback;
    void* ChainCallbackUserData;
};

static int
InputTextCallback(ImGuiInputTextCallbackData* data)
{
    InputStringCallback_UserData* user_data = (InputStringCallback_UserData*)data->UserData;
    if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
        // Resize string callback
        // If for some reason we refuse the new length (BufTextLen) and/or capacity (BufSize) we need to set them back
        // to what we want.
        std::string* str = user_data->Str;
        IM_ASSERT(data->Buf == str->c_str());
        str->resize(data->BufTextLen);
        data->Buf = (char*)str->c_str();
    } else if (user_data->ChainCallback) {
        // Forward to user callback, if any
        data->UserData = user_data->ChainCallbackUserData;
        return user_data->ChainCallback(data);
    }
    return 0;
}

} // namespace detail

bool
InputString(const char* label,
    std::string* str,
    ImGuiInputTextFlags flags = 0,
    ImGuiInputTextCallback callback = (ImGuiInputTextCallback)0,
    void* user_data = (void*)0)
{
    IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
    flags |= ImGuiInputTextFlags_CallbackResize;

    detail::InputStringCallback_UserData cb_user_data;
    cb_user_data.Str = str;
    cb_user_data.ChainCallback = callback;
    cb_user_data.ChainCallbackUserData = user_data;
    return InputText(label, (char*)str->c_str(), str->capacity() + 1, flags, detail::InputTextCallback, &cb_user_data);
}

} // namespace ImGui

namespace ui {

/**
 * Initialize ImGui context and OpenGL + GLFW backend.
 */
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

/**
 * Destroy ImGui context and OpenGL + GLFW backend.
 */
void
Deinit_ImGUI()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

/**
 * Returns all strings from `paths` which are not in `existing`.
 */
std::vector<std::string>
GetUniquePaths(const std::vector<std::string>& paths, const std::vector<std::string>& existing)
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

/**
 * Open the native file dialog for saving tilemaps.
 */
void
Dialog_SaveTileMap(Context* context)
{
    auto& state = context->state();
    // block interaction with the window while the native dialog is active
    state.interactionBlocked = true;
    context->window()->openDialog(Window::Dialog::SaveFile,
        "Save Tile Map",
        { "JSON Files", "*.json" },
        false,
        [=](bool success, std::vector<std::string>& input) {
            // file dialogs are opened on another thread.
            // microtasks are executed on the main thread.
            context->microtask([=] {
                auto& state = context->state();
                // TODO: notify user about failure
                if (success) {
                    auto path = fs::absolute(input[0]).generic_string();
                    state.tileMapSaved = true;
                    state.tileMapPath = path;
                    tile::TileMap::Save(*state.tileMap, path);
                }
                // unblock window interaction
                state.interactionBlocked = false;
            });
        });
}

/**
 * Open the native file dialog for loading tilemaps.
 */
void
Dialog_LoadTileMap(Context* context)
{
    auto& state = context->state();
    // block interaction with the window while the native dialog is active
    state.interactionBlocked = true;
    context->window()->openDialog(Window::Dialog::OpenFile,
        "Open Tile Map",
        { "JSON Files", "*.json" },
        false,
        [=](bool success, std::vector<std::string>& input) {
            // file dialogs are opened on another thread.
            // microtasks are executed on the main thread.
            context->microtask([=] {
                auto& state = context->state();
                // TODO: notify user about failure
                if (success) {
                    auto path = fs::absolute(input[0]).generic_string();
                    // assuming that tileMap is saved
                    assert(state.tileMapSaved);
                    // reset all state related to tile/tilemap.
                    state.currentTile = 0;
                    state.tileMap = tile::TileMap::Load(path);
                    state.tileMapSaved = true;
                    state.tileMapPath = path;
                    state.tileSetIndex = 0;
                }
                // unblock window interaction
                state.interactionBlocked = false;
            });
        });
}

/**
 * Open the native file dialog for loading tilesets.
 */
void
Dialog_AddTileSet(Context* context)
{
    auto& state = context->state();
    auto* tilemap = state.tileMap.get();
    // we should not open the dialog if there is no open tileMap.
    assert(tilemap != nullptr);

    // block interaction with the window while the native dialog is active
    state.interactionBlocked = true;
    context->window()->openDialog(Window::Dialog::OpenFile,
        "Select Tile Set(s)",
        { "Image Files", "*.png", "*.jpg", "*.jpeg" },
        false,
        [=](bool success, std::vector<std::string>& input) {
            // file dialogs are opened on another thread.
            // microtasks are executed on the main thread.
            context->microtask([=] {
                auto& state = context->state();
                // TODO: notify user about failure
                if (success) {
                    // we only want to add tilesets that aren't already present.
                    auto selection = GetUniquePaths(input, state.tileMap->tilesetPaths());
                    std::cout << "Added tilesets: " << std::endl;
                    for (auto& file : selection) {
                        if (!fs::exists(file))
                            continue;
                        auto* loaded = tile::TileSet::Load(file);
                        if (loaded) {
                            std::cout << file << std::endl;
                            // add the tileset to the current tilemap
                            state.tileMap->add(loaded);
                        }
                    }
                }
                // unblock window interaction
                state.interactionBlocked = false;
            });
        });
}

/**
 * Rendering for the "New Tile Map" input popup
 */
void
Render_NewTileMapPopup(Context* context, bool* is_open)
{
    if (*is_open && !ImGui::IsPopupOpen(NULL, ImGuiPopupFlags_AnyPopup)) {
        ImGui::OpenPopup("New Tile Map");
    }

    auto& state = context->state();
    auto flags = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    if (ImGui::BeginPopupModal("New Tile Map", NULL, flags)) {

        static std::string name;
        ImGui::InputString("Name", &name);

        static int selected = 2;
        const char* options[] = { "8x8", "16x16", "32x32", "64x64" };
        const char* selectedText = (selected < IM_ARRAYSIZE(options) && selected > -1) ? options[selected] : "...";
        if (ImGui::BeginCombo("Tile size", selectedText)) {
            for (int i = 0; i < IM_ARRAYSIZE(options); ++i) {
                if (ImGui::Selectable(options[i])) {
                    selected = i;
                }
            }
            ImGui::EndCombo();
        }

        if (ImGui::Button("+")) {
            // TODO: create new tilemap here
            std::cout << "Name: " << name << std::endl;
            std::cout << "Tile size: " << options[selected] << std::endl;
            ImGui::CloseCurrentPopup();
            *is_open = false;

            // Reset inputs
            name = "";
            selected = 2;
        }
        ImGui::SameLine();
        if (ImGui::Button("x")) {
            ImGui::CloseCurrentPopup();
            *is_open = false;

            // Reset inputs
            name = "";
            selected = 2;
        }

        ImGui::EndPopup();
    }
}

/**
 * Render the main UI window.
 */
void
Render_TilesetDisplayWindow(Context* context, ImGuiIO& io)
{
    // TODO(?): big function, may benefit from being split up.

    ImGui::SetNextWindowSize(ImVec2(320, 380));
    auto flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollWithMouse |
                 ImGuiWindowFlags_NoScrollbar;
    if (!ImGui::Begin("Tile Set", NULL, flags))
        return ImGui::End();

    auto& state = context->state();
    auto* window = context->window();
    auto& style = ImGui::GetStyle();

    // Editor UI body

    auto* tilemap = state.tileMap.get();
    { // tileset selection
        if (tilemap != nullptr) {
            std::vector<std::string>& tspaths = tilemap->tilesetPaths();

            // current_tileset_path is displayed in the selection box
            // there should be no way to reach an invalid tileset index.
            const char* current_tileset_path = "Select a tile set...";
            if (state.tileSetIndex < tspaths.size()) {
                current_tileset_path = tspaths[state.tileSetIndex].c_str();
            } else {
                current_tileset_path = "Select a tile set...";
            }

            if (ImGui::BeginCombo("Tilesets", current_tileset_path)) {
                for (size_t i = 0; i < tspaths.size(); ++i) {
                    if (ImGui::Selectable(tspaths[i].c_str())) {
                        state.tileSetIndex = i;
                    }
                }
                ImGui::EndCombo();
            }
            ImGui::SameLine();

            if (ImGui::SmallButton("+") && !ImGui::IsPopupOpen(NULL, ImGuiPopupFlags_AnyPopup)) {
                spdlog::info("Add tileset");
                Dialog_AddTileSet(context);
                state.tileMapSaved = false;
            }
        }
    }
    { // tileset display
        if (tilemap != nullptr) {
            std::vector<tile::TileSet*> tilesets = tilemap->tilesets();

            if (state.tileSetIndex < tilesets.size()) {
                // TODO: always zoom into current middle of the view instead of origin
                // It's actually a bit of work:
                // #1:  Render around CENTER, not BOTTOM_LEFT, so that [0, 0] in display
                //      corresponds to tile at middle of current tileset.
                // #2:  Zoom should be applied to the view, and not the tileset width/height.
                // #3:  All calculations need to be done before drawing.
                // #4:  (?) Correct the UVs of the image, so that it doesn't need flip when drawing.
                // #5:  Use the same algorithm as in src/camera.cpp:
                //          1. Get mouse position in display before zoom.
                //          2. Apply zoom.
                //          3. Get mouse position in display after zoom.
                //          4. Translate by (#1 - #3).

                static ImVec2 offset(0.0f, 0.0f);
                static float zoom = 1.0f;

                // Get the ImGui cursor in screen space
                ImVec2 display_p0 = ImGui::GetCursorScreenPos();
                // Get size of the tileset display
                ImVec2 display_sz = ImGui::GetContentRegionAvail();
                display_sz.x = (display_sz.x < 50.0f ? 50.0f : display_sz.x);
                display_sz.y = (display_sz.y < 50.0f ? 50.0f : (display_sz.y > 320.0f ? 320.0f : display_sz.y));
                ImVec2 display_p1 = ImVec2(display_p0.x + display_sz.x, display_p0.y + display_sz.y);

                // Draw border and background color
                ImGuiIO& io = ImGui::GetIO();
                ImDrawList* draw_list = ImGui::GetWindowDrawList();
                draw_list->AddRectFilled(display_p0, display_p1, IM_COL32(50, 50, 50, 255));
                draw_list->AddRect(display_p0, display_p1, IM_COL32(255, 255, 255, 255));

                // Create an invisible button which will catch inputs
                // on the tileset display
                auto display_rect_flags = ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight |
                                          ImGuiButtonFlags_MouseButtonMiddle;
                ImGui::InvisibleButton("display", display_sz, display_rect_flags);
                const bool is_hovered = ImGui::IsItemHovered();
                const bool is_active = ImGui::IsItemActive();
                // Calculate origin, then use it to calculate where the mouse cursor is
                const ImVec2 origin(display_p0.x + offset.x, display_p0.y + offset.y);
                const ImVec2 mouse_pos_in_display(io.MousePos.x - origin.x, io.MousePos.y - origin.y);

                // Pan the tileset display
                if (is_active && ImGui::IsMouseDragging(ImGuiMouseButton_Middle, 0.0f)) {
                    offset.x += io.MouseDelta.x;
                    offset.y += io.MouseDelta.y;
                }
                // Zoom into tileset display
                if (is_hovered) {
                    // Minimum zoom = x0.1 (near camera limit)
                    // Maximum zoom = x5.0 (it can fit an image with 1600px width)
                    zoom = std::clamp(zoom - io.MouseWheel / 10.0f, 0.1f, 5.0f);
                }

                // Clip what we're drawing next
                draw_list->PushClipRect(display_p0, display_p1, true);

                // Get selected tileset image
                auto* currentTileSet = tilesets[state.tileSetIndex];
                auto& currentTileSetAtlas = currentTileSet->atlas();

                // Draw the tileset
                // We're drawing it flipped on both axes due to how the image data is loaded and handled inside ImGui.
                // TODO: investigate a cleaner solution
                auto tileset_p0 = ImVec2(origin.x, origin.y - (currentTileSetAtlas.height() / zoom) + display_sz.y);
                auto tileset_p1 = ImVec2(origin.x + (currentTileSetAtlas.width() / zoom), origin.y + display_sz.y);
                draw_list->AddImage(
                    (void*)(currentTileSetAtlas.handle()), tileset_p0, tileset_p1, ImVec2(0, 1), ImVec2(1, 0));

                // Get the currently hovered + active tile
                // And set the current tile on click
                auto zoomed_tileSize = tilemap->tileSize() / zoom;
                if (is_hovered) {
                    // calculate mouse pos on atlas
                    auto mx = (mouse_pos_in_display.x) * zoom;
                    auto my = -(mouse_pos_in_display.y - display_sz.y) * zoom;

                    // check if mouse is intersecting atlas
                    auto atlasW = (float)currentTileSetAtlas.width();
                    auto atlasH = (float)currentTileSetAtlas.height();
                    if (mx > 0 && mx < atlasW && my > 0 && my < atlasH) {
                        auto tile_column = std::floor(mx / tilemap->tileSize());
                        auto tile_row = std::floor(my / tilemap->tileSize());

                        // draw tile highlight
                        auto hover_p0 = ImVec2(origin.x + (zoomed_tileSize * tile_column),
                            origin.y + display_sz.y - (zoomed_tileSize * tile_row));
                        auto hover_p1 = ImVec2(hover_p0.x + zoomed_tileSize, hover_p0.y - zoomed_tileSize);
                        draw_list->AddRectFilled(hover_p0, hover_p1, IM_COL32(120, 120, 120, 100));

                        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                            // calculate tile ID
                            auto tilesPerRow = atlasW / (float)tilemap->tileSize();
                            auto tileSetId = state.tileSetIndex;
                            // X and Y are reversed because of UV flip when drawing tileset atlas (see above @AddImage)
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
                    auto uvPerRow = (currentTileSetAtlas.width() / tilemap->tileSize());
                    auto tile_x = zoomed_tileSize * (currentTileId / uvPerRow);
                    auto tile_y = zoomed_tileSize * (currentTileId % uvPerRow);
                    auto active_p0 = ImVec2(origin.x + tile_x, origin.y + display_sz.y - tile_y);
                    auto active_p1 = ImVec2(active_p0.x + zoomed_tileSize, active_p0.y - zoomed_tileSize);
                    draw_list->AddRect(active_p0, active_p1, IM_COL32(84, 229, 255, 255), 0.0f, 15, 3.0f);
                }

                draw_list->PopClipRect();
            }
        }
    }

    ImGui::End();
}

/**
 * Helper for saving current TileMap.
 * Checks if the TileMap has a save path already, and saves there. Otherwise, opens a native save file dialog.
 */
void
Save_TileMap(Context* context)
{
    auto& state = context->state();
    if (state.tileMapPath.empty() && !ImGui::IsPopupOpen(NULL, ImGuiPopupFlags_AnyPopup)) {
        Dialog_SaveTileMap(context);
    } else {
        state.tileMapSaved = true;
        tile::TileMap::Save(*state.tileMap, state.tileMapPath);
    }
}

void
Render_EditorMenuBar(Context* context)
{
    auto& state = context->state();
    auto* window = context->window();
    // Editor UI menu
    // TODO: render this as a MainMenuBar instead of menu bar on this window
    if (ImGui::BeginMainMenuBar()) {

        // New Tile Map
        // Menu->New or CTRL+N
        // Popup must not be open
        static bool new_tmp_open = false;
        if ((ImGui::MenuItem("New", "CTRL+N") || window->shortcut(Window::Modifier::CONTROL, GLFW_KEY_N)) &&
            !ImGui::IsPopupOpen(NULL, ImGuiPopupFlags_AnyPopup)) {
            // if the current tile map has unsaved changes
            if (!state.tileMapSaved) {
                context->confirm("Save unsaved progress?", [&](bool choice) {
                    if (choice) {
                        Save_TileMap(context);
                    }
                    new_tmp_open = true;
                });
            } else {
                // otherwise just open the new tilemap popup
                new_tmp_open = true;
            }
        }
        Render_NewTileMapPopup(context, &new_tmp_open);

        // Save Tile Map
        // Menu->Save or CTRL+S
        if (ImGui::MenuItem("Save", "CTRL+S") || window->shortcut(Window::Modifier::CONTROL, GLFW_KEY_S)) {
            // only saves if we haven't already saved.
            if (!state.tileMapSaved) {
                // Only opens native file dialog if we don't have a stored save path
                Save_TileMap(context);
            }
        }

        // Save Tile Map As
        // Menu->Save As, no shortcut
        if ((ImGui::MenuItem("Save As")) && !ImGui::IsPopupOpen(NULL, ImGuiPopupFlags_AnyPopup)) {
            // Always opens native save file dialog
            Dialog_SaveTileMap(context);
        }

        // Open Tile Map
        // Menu->Open or CTRL+O
        if ((ImGui::MenuItem("Open", "CTRL+O") || window->shortcut(Window::Modifier::CONTROL, GLFW_KEY_O)) &&
            !ImGui::IsPopupOpen(NULL, ImGuiPopupFlags_AnyPopup)) {
            if (!state.tileMapSaved) {
                context->confirm("Save unsaved progress?", [&](bool choice) {
                    if (choice) {
                        Save_TileMap(context);
                    }
                    Dialog_LoadTileMap(context);
                });
            } else {
                Dialog_LoadTileMap(context);
            }
        }
        ImGui::EndMainMenuBar();
    }
}

void
Render_Popups(Context* context)
{
    auto& state = context->state();
    {
        // confirm dialog is triggered with Context::confirm
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
    Render_EditorMenuBar(this);
    Render_TilesetDisplayWindow(this, io);
    Render_Popups(this);

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
Context::forceSaveDialog(std::function<void()> then)
{
    if (ImGui::BeginPopupModal("confirm_dialog", NULL) || ImGui::BeginPopupModal("interaction_blocker", NULL)) {
        ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    // TODO: de-duplicate this code
    auto& state = this->state();
    if (state.tileMapPath.empty()) {
        auto& state = this->state();
        // block interaction with the window while the native dialog is active
        state.interactionBlocked = true;
        this->window()->openDialog(Window::Dialog::SaveFile,
            "Save Tile Map",
            { "JSON Files", "*.json" },
            false,
            [=](bool success, std::vector<std::string>& input) {
                // file dialogs are opened on another thread.
                // microtasks are executed on the main thread.
                this->microtask([=] {
                    auto& state = this->state();
                    // TODO: notify user about failure
                    if (success) {
                        auto path = fs::absolute(input[0]).generic_string();
                        state.tileMapSaved = true;
                        state.tileMapPath = path;
                        tile::TileMap::Save(*state.tileMap, path);
                    }
                    // unblock window interaction
                    state.interactionBlocked = false;
                    then();
                });
            });
    } else {
        state.tileMapSaved = true;
        tile::TileMap::Save(*state.tileMap, state.tileMapPath);
        then();
    }
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