
#include "pch.h"
#include "gfx/gfx.hpp"
#include "ui.hpp"
#include "tile.hpp"

/*

TODO: put all in src/gfx under "gfx" namespace.
TODO: line rendering

TODO: automatically resize tilemap as needed by growing/shrinking the arrays
TODO: painting tiles with pencil-like tool (make this generic to allow for other tools?)
-> tilemap[hovered_tile.x, hovered_tile.y] = currentTile

*/

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
        // modifiers -> key -> action
        if (modifiers == 0 /* none */) {
            if (key == GLFW_KEY_ESCAPE)
                return window.close();
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

        auto* tilemap = context.state().tileMap.get();
        if (tilemap != nullptr) {
            // starting row/col so that the tilemap is centered by default
            auto srow = 0.5 + ((float)tilemap->rows() / -2.f);
            auto scol = 0.5 + ((float)tilemap->columns() / -2.f);
            glm::vec3 tileScale = { (float)tilemap->tileSize() / 2.f, (float)tilemap->tileSize() / 2.f, 1.f };
            for (size_t row = 0; row < tilemap->rows(); ++row) {
                for (size_t column = 0; column < tilemap->columns(); ++column) {
                    auto tile = tilemap->get(column, row);
                    auto tileset = tilemap->tileset(tile);
                    auto uv = tilemap->uv(tile);
                    auto model = glm::translate(glm::mat4(1), { (scol + column) * 32.f, (srow + row) * 32.f, 0.f });
                    model = glm::scale(model, tileScale);
                    renderer.submit(&tileset->atlas(), uv, model);
                }
                continue;
            }
        }
        renderer.render(camera);

        context.render();

        /* Swap front and back buffers */
        window.swapBuffers();
    }

    return 0;
}
