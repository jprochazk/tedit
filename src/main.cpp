
#include "pch.h"
#include "gfx/gfx.hpp"
#include "ui.hpp"
#include "tile.hpp"

/*

TODO: store tiles 2D array instead of 1D
TODO: automatically resize tilemap as needed by growing/shrinking the arrays
TODO: painting tiles with pencil-like tool (make this generic to allow for other tools?)
-> tilemap[hovered_tile.x, hovered_tile.y] = currentTile

*/

int
main(void)
{
    Window window("Test Window", 1600, 900);
    Camera camera(&window);
    Renderer renderer;

    ui::Context context(&window);

    auto& state = context.state();
    state.tileMap = tile::TileMap::Load("SAMPLE_MAP.json");
    state.tileMapPath = "SAMPLE_MAP.json";

    // TODO: maybe refactor this a bit
    bool is_dragging = false;
    glm::dvec2 initialPosition(0, 0);
    glm::dvec2 panStart(0, 0);

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
        glm::dvec2 mouse(0, 0);
        glfwGetCursorPos(window.handle(), &mouse[0], &mouse[1]);
        camera.zoom(yoffset, mouse);
    });
    window.addResizeListener([&](int width, int height) { camera.resize(width, height); });

    /* Loop until the user closes the window */
    while (!window.shouldClose()) {
        /* Poll for and process events */
        window.pollInput();
        context.poll();

        renderer.begin(camera);

        // TODO: test this properly
        auto* tilemap = context.state().tileMap.get();
        if (tilemap != nullptr) {
            glm::vec3 tileScale = { (float)tilemap->tileSize() / 2.f, (float)tilemap->tileSize() / 2.f, 1.f };
            for (size_t row = 0; row < tilemap->rows(); ++row) {
                for (size_t column = 0; column < tilemap->columns(); ++column) {
                    auto tile = tilemap->get(column, row);
                    auto tileset = tilemap->tileset(tile);
                    auto uv = tilemap->uv(tile);
                    auto model = glm::translate(glm::mat4(1), { column * 32.f, row * 32.f, 0.f });
                    model = glm::scale(model, tileScale);
                    renderer.draw(&tileset->atlas(), uv, model);
                }
                continue;
            }
        }
        renderer.flush();

        context.render();

        /* Swap front and back buffers */
        window.swapBuffers();
    }

    return 0;
}
