#include "pch.h"

#ifndef TEDIT_UI_
#define TEDIT_UI_

#include <imgui.h>
#include "task.hpp"

// forward declarations
class Window;
namespace tile {
using Tile = uint16_t;
class TileSet;
class TileMap;
}

namespace ui {

struct ConfirmDialog
{
    const char* text = "";
    std::function<void(bool)> callback = nullptr;
    bool done = true;
}; // struct ConfirmDialog

struct ContextState
{
    std::string tileMapPath;
    bool tileMapSaved = true;
    std::unique_ptr<tile::TileMap> tileMap = nullptr;
    tile::Tile currentTile = 0;
    uint16_t tileSetIndex = 0;
    bool hasMouseFocus = false;
    bool hasKeyboardFocus = false;
    ConfirmDialog currentDialog = {};
    bool interactionBlocked = false;
}; // struct ContextState

class Context
{
public:
    Context(Window* window);
    ~Context();

    void render();

    Window* window();
    const Window* window() const;
    ContextState& state();
    const ContextState& state() const;

    void confirm(const char* text, std::function<void(bool)> callback);

    void microtask(Task&& task);
    void poll();

private:
    Window* window_;
    ContextState state_;
    TaskQueue tasks_;
}; // class Context

}; // namespace ui

#endif // TEDIT_UI_
