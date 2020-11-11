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
    NEW = 1 << 0,
    SAVE = 1 << 1,
    OPEN = 1 << 2,
}; // enum Dialog

struct ContextState
{
    tile::TileMap* tileMap = nullptr;
    tile::Tile currentTile = 0;
    uint16_t tileSetIndex = 0;
    bool hasMouseFocus = false;
    bool hasKeyboardFocus = false;
}; // struct ContextState

class Context
{
public:
    Context(Window* window);
    ~Context();

    void render();

    void dialog(Dialog dialog);

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
