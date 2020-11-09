
#include "pch.h"
#include "gfx/gfx.hpp"
#include "ui.hpp"
#include "tile.hpp"

int
main(void)
{
    Window window("Test Window", 1600, 900);
    Renderer renderer;

    ui::Context context(&window);

    window.addKeyListener([&context](int key, int action, int modifiers) {
        if (modifiers & GLFW_MOD_CONTROL == GLFW_MOD_CONTROL && action == GLFW_PRESS) {
            if (key == GLFW_KEY_N)
                return context.dialog(ui::Dialog::NEW);
            if (key == GLFW_KEY_S)
                return context.dialog(ui::Dialog::SAVE);
            if (key == GLFW_KEY_O)
                return context.dialog(ui::Dialog::OPEN);
        }
    });

    auto tilemap = tile::TileMap::Load("SAMPLE_MAP.json");
    assert(!!tilemap);
    std::cout << fmt::format(
                   "{}, {}, {}, {}", tilemap->name(), tilemap->columns(), tilemap->rows(), tilemap->tileSize())
              << std::endl;
    for (uint32_t row = 0; row < tilemap->rows(); ++row) {
        for (uint32_t column = 0; column < tilemap->columns(); ++column) {
            uint32_t tile = tilemap->get(column, row);
            std::cout << fmt::format("({}, {})", tile::TileSetId(tile), tile::TileId(tile)) << " ";
        }
        std::cout << std::endl;
    }

    tilemap->set(1, 1, 0b00100011);

    std::cout << std::endl;
    for (uint32_t row = 0; row < tilemap->rows(); ++row) {
        for (uint32_t column = 0; column < tilemap->columns(); ++column) {
            uint32_t tile = tilemap->get(column, row);
            std::cout << fmt::format("({}, {})", tile::TileSetId(tile), tile::TileId(tile)) << " ";
        }
        std::cout << std::endl;
    }

    try {
        tile::TileMap::Save(*tilemap, "SAMPLE_MAP2.json");
    } catch (std::exception& ex) {
        std::cerr << "[ERROR]: " << ex.what() << std::endl;
    }

    /* Loop until the user closes the window */
    while (!window.shouldClose()) {
        /* Poll for and process events */
        window.pollInput();

        auto state = context.state();

        auto model = glm::scale(glm::mat4(1), { 32.f, 32.f, 1.f });

        renderer.begin(window);
        // renderer.draw(tileset->atlas(), uv, model);
        renderer.flush();

        context.render();

        /* Swap front and back buffers */
        window.swapBuffers();
    }

    return 0;
}
