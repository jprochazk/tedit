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

enum Dialog : uint8_t
{
    NONE = 0,
    NEW = 1 << 0,
    SAVE = 1 << 1,
    SAVEAS = 1 << 2,
    OPEN = 1 << 3,
    TILESET = 1 << 4
}; // enum Dialog

struct ContextState
{
    tile::TileMap* tileMap = nullptr;
    tile::Tile currentTile = 0;
    uint16_t tileSetIndex = 0;
    bool hasMouseFocus = false;
    bool hasKeyboardFocus = false;
    Dialog currentDialog = Dialog::NONE;
    bool tileMapSaved = false;
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

    void microtask(Task&& task);
    void poll();

private:
    Window* window_;
    ContextState state_;
    TaskQueue tasks_;
}; // class Context

}; // namespace ui

#endif // TEDIT_UI_
