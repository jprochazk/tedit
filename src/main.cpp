
#include "pch.h"
#include "gfx/gfx.hpp"
#include "ui.hpp"

// TODO: tileset
// TODO: tilemap

#include "tile.hpp"

int
main(void)
{
    Window window("Test Window", 1600, 900);
    Renderer renderer;

    ui::Context context(&window);

    window.addKeyListener([&context](int key, int action, int modifiers) {
        if (modifiers & GLFW_MOD_CONTROL == GLFW_MOD_CONTROL && action == GLFW_PRESS) {
            if (key == GLFW_KEY_S)
                return context.openDialog(ui::Dialog::SAVE);
            if (key == GLFW_KEY_O)
                return context.openDialog(ui::Dialog::OPEN);
        }
    });

    auto tileset = std::make_shared<tile::TileSet>("TILESET.png", 32);
    // tile::TileMap tilemap(32, 32, 32);
    // tilemap.add(tileset);
    auto tile = tile::Tile(0);
    auto tileSize = tileset->getTileSize();

    auto _uv = tileset->uv(tile);
    spdlog::info("vec4({}, {}, {}, {})", _uv.x, _uv.y, _uv.z, _uv.w);

    /* Loop until the user closes the window */
    while (!window.shouldClose()) {
        /* Poll for and process events */
        window.pollInput();

        auto state = context.getState();

        auto uv = tileset->uv(tile);
        auto model = glm::scale(glm::mat4(1), { (float)tileSize * 10.f, (float)tileSize * 10.f, 1.f });

        renderer.begin(window);
        renderer.draw(tileset->getImage(), uv, model);
        renderer.flush();

        context.render();

        /* Swap front and back buffers */
        window.swapBuffers();
    }

    return 0;
}
