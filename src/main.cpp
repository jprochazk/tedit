
#include "pch.h"
#include "gfx/gfx.hpp"
#include "ui.hpp"
#include "tile.hpp"

/*

TODO: new tilemap menu button implementation
TODO: document controls somewhere

TODO: resize by dragging edges
TODO: layers
TODO: other tools
* brush
* fill
* line
* stamp
* copy/paste tiles
TODO: display some tilemap info
* columns/rows
* tilesize
*

*/

void
draw_tiles(gfx::Renderer* renderer, tile::TileMap* tilemap)
{
    // TODO: center camera on tilemap instead of centering tilemap on camera.
    float tileSize = (float)tilemap->tileSize();
    float halfTileSize = tileSize / 2.f;
    glm::vec3 tileScale = { halfTileSize, halfTileSize, 1.f };
    for (size_t row = 0; row < tilemap->rows(); ++row) {
        for (size_t column = 0; column < tilemap->columns(); ++column) {
            auto tile = (*tilemap)(column, row);
            auto tileset = tilemap->tileset(tile);
            auto uv = tilemap->uv(tile);
            auto model = glm::translate(glm::mat4(1), { halfTileSize + column * 32.f, halfTileSize + row * 32.f, 0.f });
            model = glm::scale(model, tileScale);
            renderer->submit(&tileset->atlas(), uv, model);
        }
        continue;
    }
}

void
draw_grid(gfx::Renderer* renderer, glm::vec<2, size_t> mapSize, size_t tileSize, glm::vec2 mouse)
{
    auto width = mapSize.x * tileSize;
    auto height = mapSize.y * tileSize;

    // highlight
    auto mcol = std::floor(mouse.x / tileSize);
    auto mrow = std::floor(mouse.y / tileSize);

    // mouse is in tilemap bounds
    if (mcol >= 0 && mcol < mapSize.x && mrow >= 0 && mrow < mapSize.y) {
        auto halfTileSize = (float)tileSize / 2.f;
        glm::vec3 tileScale = { halfTileSize, halfTileSize, 1.f };
        auto model = glm::translate(glm::mat4(1), { halfTileSize + mcol * 32.f, halfTileSize + mrow * 32.f, 0.f });
        model = glm::scale(model, tileScale);
        renderer->submit({ 120.f / 255.f, 120.f / 255.f, 120.f / 255.f, 100.f / 255.f }, model);
    }

    // horizontal lines
    for (size_t row = 0; row < mapSize.y + 1; ++row) {
        renderer->submit({ 0.0f, row * tileSize }, { width, row * tileSize }, 1.f);
    }
    // vertical lines
    for (size_t column = 0; column < mapSize.x + 1; ++column) {
        renderer->submit({ column * tileSize, 0.0f }, { column * tileSize, height }, 1.f);
    }
}

int
main(void)
{
    Window window("Test Window", 1600, 900);
    gfx::Camera camera(&window);
    gfx::Renderer renderer;

    ui::Context context(&window);

    // NOTE: temporarily load sample map on startup
    // TODO: remove this
    auto& state = context.state();
    state.tileMap = tile::TileMap::Load("SAMPLE_MAP.json");
    state.tileMapSaved = true;
    state.tileMapPath = "SAMPLE_MAP.json";

    // TODO: maybe refactor this a bit
    bool is_dragging = false;
    glm::dvec2 initialPosition(0, 0);
    glm::dvec2 panStart(0, 0);

    // setup input handling
    window.addMouseMoveListener([&](double mouseX, double mouseY) {
        if (context.state().hasMouseFocus) {
            return;
        }

        if (is_dragging) {
            glm::vec2 delta = initialPosition + (panStart - glm::dvec2(mouseX, -mouseY)) / glm::dvec2(camera.zoom());
            camera.move(delta);
            return;
        }
    });
    window.addMouseButtonListener([&](int button, int action, int modifiers) {
        if (context.state().hasMouseFocus) {
            return;
        }
        if (modifiers == 0) {
            if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
                if (action == GLFW_PRESS) {
                    initialPosition = camera.pos();
                    glfwGetCursorPos(window.handle(), &panStart.x, &panStart.y);
                    panStart.y = -panStart.y;
                    is_dragging = true;
                } else if (action == GLFW_RELEASE) {
                    is_dragging = false;
                }
            }
        }
    });
    window.addKeyListener([&](int key, int action, int modifiers) {
        if (context.state().hasKeyboardFocus) {
            return;
        }
        // action -> modifiers -> key
        if (action == GLFW_PRESS) {
            if (modifiers == 0 /* none */) {
                if (key == GLFW_KEY_ESCAPE) {
                    window.close();
                    return;
                }
                if (key == GLFW_KEY_L) {
                    state.tileMap->grow(tile::TileMap::Direction::Right, 1);
                    return;
                }
                if (key == GLFW_KEY_J) {
                    state.tileMap->grow(tile::TileMap::Direction::Left, 1);
                }
                if (key == GLFW_KEY_I) {
                    state.tileMap->grow(tile::TileMap::Direction::Top, 1);
                    return;
                }
                if (key == GLFW_KEY_K) {
                    state.tileMap->grow(tile::TileMap::Direction::Bottom, 1);
                }
            }
        }
    });
    window.addScrollListener([&](double xoffset, double yoffset) {
        if (context.state().hasMouseFocus) {
            return;
        }
        // glm::dvec2 mouse(0, 0);
        // glfwGetCursorPos(window.handle(), &mouse[0], &mouse[1]);
        // camera.zoom(yoffset, mouse);
        camera.zoom(yoffset);
    });
    window.addResizeListener([&](int width, int height) { camera.resize(width, height); });

    /* Loop until the user closes the window */
    while (!window.shouldClose()) {
        /* Poll for and process events */
        window.pollInput();
        context.poll();

        glm::dvec2 mouse;
        glfwGetCursorPos(window.handle(), &mouse.x, &mouse.y);
        auto mouseInWorld = camera.world(mouse);

        auto* tilemap = context.state().tileMap.get();
        if (tilemap != nullptr) {
            glm::vec2 mapSize = { tilemap->columns(), tilemap->rows() };
            if (glfwGetMouseButton(window.handle(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
                auto tileSize = tilemap->tileSize();
                auto col = std::floor(mouseInWorld.x / tileSize);
                auto row = std::floor(mouseInWorld.y / tileSize);
                if (col >= 0 && col < mapSize.x && row >= 0 && row < mapSize.y) {
                    auto& tile = (*tilemap)(col, row);
                    if (tile != state.currentTile) {
                        tile = state.currentTile;
                        state.tileMapSaved = false;
                    }
                }
            }

            draw_tiles(&renderer, tilemap);
            draw_grid(&renderer, mapSize, tilemap->tileSize(), mouseInWorld);
        }

        renderer.render(camera);
        context.render();

        /* Swap front and back buffers */
        window.swapBuffers();
    }

    return 0;
}
