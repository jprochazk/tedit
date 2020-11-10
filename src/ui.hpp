#include "pch.h"
#ifndef TEDIT_UI_
#    define TEDIT_UI_

#    include <imgui.h>

// forward declarations
class Window;
namespace tile {
using Tile = uint8_t;
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
    std::shared_ptr<tile::TileMap> tileMap;
    tile::Tile currentTile;
    tile::TileSet* tileSet;
}; // struct ContextState

class Context
{
public:
    Context(Window* window);
    ~Context();

    void render();

    void dialog(Dialog dialog);

    ContextState& state();
    const ContextState& state() const;

private:
    Window* window_;
    ContextState state_;
}; // class Context

}; // namespace ui

#endif // TEDIT_UI_
