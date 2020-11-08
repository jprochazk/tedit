#ifndef TEDIT_UI_
#define TEDIT_UI_

#include "pch.h"
#include <imgui.h>

// forward declarations
class Window;
namespace tile {
using Tile = uint8_t;
class TileMap;
}

namespace ui {

enum Dialog : uint8_t
{
    SAVE = 1 << 0,
    OPEN = 1 << 1
}; // enum Dialog

struct ContextState
{
    std::shared_ptr<tile::TileMap> tileMap;
    tile::Tile currentTile;
}; // struct ContextState

class Context
{
public:
    Context(Window* window);
    ~Context();

    void render();

    void openDialog(Dialog dialog);

    ContextState& getState();

    const ContextState& getState() const;

    Window* window;
    ContextState state;
}; // class Context

}; // namespace ui

#endif // TEDIT_UI_
