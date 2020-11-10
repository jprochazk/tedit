
#include "pch.h"
#include "gfx/gfx.hpp"
#include "ui.hpp"
#include "tile.hpp"

/*

TODO: display list of tilesets as a select box
-> currentTileSet = selected
TODO: add/remove tileset (buttons in the select box)

TODO: display tileset in editor UI using DrawList api
* pan and zoom by holding/scrolling middle mouse button
* select tile by clicking left mouse button
-> TileId(currentTile)
-> TileSetId(currentTile, currentTileMap->tileSetId(currentTileSet))

TODO: camera
* 2d orthographic top-down
* pan and zoom by holding/scrolling middle mouse button

TODO: painting tiles with pencil-like tool (make this generic to allow for other tools?)
-> tilemap[hovered_tile.x, hovered_tile.y] = currentTile

TODO: new/save/save as/load dialogs for tilemaps
* new       -> tilemap name, starting size, tile size

* save      -> if new tilemap (not saved)
                -> open native file dialog
                -> confirm if file already exists
            -> else save to previous save location

* save as   -> open native file dialog
            -> confirm if file already exists

* load      -> open native file dialog

TODO: resize tilemap
* either manually add rows/columns (tedious)
* or automatically update tilemap bounds as new tiles are added/removed

*/

int
main(void)
{
    Window window("Test Window", 1600, 900);
    Renderer renderer;

    ui::Context context(&window);

    window.addKeyListener([&context, &window](int key, int action, int modifiers) {
        if (action == GLFW_PRESS) {
            if (key == GLFW_KEY_ESCAPE)
                return window.close();
            if (modifiers & GLFW_MOD_CONTROL == GLFW_MOD_CONTROL) {
                if (key == GLFW_KEY_N)
                    return context.dialog(ui::Dialog::NEW);
                if (key == GLFW_KEY_S)
                    return context.dialog(ui::Dialog::SAVE);
                if (key == GLFW_KEY_O)
                    return context.dialog(ui::Dialog::OPEN);
            }
        }
    });

    auto tilemap = (*tile::TileMap::Load("SAMPLE_MAP.json"));

    auto& state = context.state();

    /* Loop until the user closes the window */
    while (!window.shouldClose()) {
        /* Poll for and process events */
        window.pollInput();

        renderer.begin(window);

        // TODO: test this properly
        // for (size_t row = 0; row < tilemap.rows(); ++row) {
        //    for (size_t column = 0; column < tilemap.columns(); ++column) {
        auto tile = tilemap.get(0, 0);
        auto tileset = tilemap.tileset(tile);
        auto uv = tilemap.uv(tile);
        auto model = glm::mat4(1); // glm::translate(glm::mat4(1), { 0 * 64.f, 0 * 64.f, 0.f });
        model = glm::scale(model, { 32.f, 32.f, 1.f });
        std::cout << glm::to_string(uv) << std::endl;
        renderer.draw(&tileset->atlas(), uv, model);
        //    }
        //    continue;
        //}
        // renderer.draw(tileset->atlas(), uv, model);
        renderer.flush();

        context.render();

        /* Swap front and back buffers */
        window.swapBuffers();
    }

    return 0;
}
