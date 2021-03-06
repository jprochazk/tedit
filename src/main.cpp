
#include "pch.h"
#include "gfx/gfx.hpp"
#include "ui.hpp"
#include "tile.hpp"

/*

** Post-MVP **
TODO: position camera on tilemap center at tilemap load
TODO: sessions (store camera position, open tilemap, selected tile, selected tool, etc)
TODO: recent files
TODO: undo/redo (CTRL+Z/CTRL+SHIFT+Z)
TODO: layers
TODO: other tools
    * RESIZE -> hover tilemap edge + drag LMB ... once implemented, remove key controls
    * BRUSH -> hold LMB
    * FILL -> click LMB
    * LINE -> click LMB (start) + click LMB (end)
    * STAMP -> select tiles in tileset -> click LMB
    * SELECT TILES ->
        * DELETE -> DEL
        * COPY -> CTRL+C
        * PASTE -> CTRL+V
TODO: display some tilemap info
    * columns/rows
    * tilesize
TODO: various scattered TODOs
TODO: variable tilesize

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
            if (tileset == nullptr)
                continue;
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

#if 0 // def _WIN32
int WINAPI
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
#else
int
main(int argc, char** argv)
#endif
{
    Window window("Test Window", 1600, 900);
    gfx::Camera camera(&window);
    gfx::Renderer renderer;

#ifdef _WIN32
    CHAR path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    std::cout << path << std::endl;
    auto executablePath = fs::path(path).parent_path().generic_string();
#else
    auto executablePath = fs::path(argv[0]).parent_path().generic_string();
#endif
    ui::Context context(&window, executablePath);

    auto& state = context.state();
    window.setTitle(fmt::format("TEdit", state.tileMapPath));

    // TODO: maybe refactor this a bit
    // put input handling somewhere separate
    // + each possible input should have an associated command
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
                    if (state.tileMap != nullptr && !state.tileMapSaved) {
                        context.confirm("Save unsaved progress?", [&](bool choice) {
                            if (choice) {
                                context.forceSaveDialog([&] { window.close(); });
                            } else {
                                window.close();
                            }
                        });
                    } else {
                        window.close();
                    }
                    return;
                }
                if (key == GLFW_KEY_L) {
                    state.tileMap->resize(tile::TileMap::Direction::Right, 1);
                    return;
                }
                if (key == GLFW_KEY_J) {
                    state.tileMap->resize(tile::TileMap::Direction::Left, 1);
                }
                if (key == GLFW_KEY_I) {
                    state.tileMap->resize(tile::TileMap::Direction::Top, 1);
                    return;
                }
                if (key == GLFW_KEY_K) {
                    state.tileMap->resize(tile::TileMap::Direction::Bottom, 1);
                    return;
                }
            }
            if (modifiers == GLFW_MOD_SHIFT) {
                if (key == GLFW_KEY_L) {
                    state.tileMap->resize(tile::TileMap::Direction::Right, -1);
                    return;
                }
                if (key == GLFW_KEY_J) {
                    state.tileMap->resize(tile::TileMap::Direction::Left, -1);
                }
                if (key == GLFW_KEY_I) {
                    state.tileMap->resize(tile::TileMap::Direction::Top, -1);
                    return;
                }
                if (key == GLFW_KEY_K) {
                    state.tileMap->resize(tile::TileMap::Direction::Bottom, -1);
                    return;
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

            if (!state.hasMouseFocus) {
                if (glfwGetMouseButton(window.handle(), GLFW_MOUSE_BUTTON_LEFT)) {
                    auto tileSize = tilemap->tileSize();
                    auto col = std::floor(mouseInWorld.x / tileSize);
                    auto row = std::floor(mouseInWorld.y / tileSize);
                    if (col >= 0 && col < mapSize.x && row >= 0 && row < mapSize.y) {
                        auto& tile = (*tilemap)(col, row);

                        tile::Tile currentTile = state.currentTile;
                        // if we're holding shift, erase
                        if (glfwGetKey(window.handle(), GLFW_KEY_LEFT_SHIFT) ||
                            glfwGetKey(window.handle(), GLFW_KEY_RIGHT_SHIFT)) {
                            currentTile = tile::Tile(-1);
                        }
                        // otherwise, paint
                        if (tile != currentTile) {
                            tile = currentTile;

                            if (state.tileMapSaved) {
                                state.tileMapSaved = false;
                                window.setTitle(fmt::format("TEdit - {}*", state.tileMapPath));
                            }
                        }
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
