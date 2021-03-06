# tedit

This repository hosts a Tile-based Map Editor written in C++. 

It uses:
* [`GLFW`](https://www.glfw.org/) for window and input handling
* [`OpenGL`](https://www.opengl.org/) for rendering
* [`ImGUI`](https://github.com/ocornut/imgui) for UI rendering
* [`JSON for Modern C++`](https://github.com/nlohmann/json) for serialization/deserialization
* [`STB`](https://github.com/nothings/stb) for image loading
* [`GLM`](https://glm.g-truc.net/0.9.9/index.html) for vector mathematics
* [`Portable File Dialogs`](https://github.com/samhocevar/portable-file-dialogs) for native cross-platform save/open file dialogs

## Features

* Tile map display
* Tile map save/load to JSON
* Selecting tiles from a tile atlas
* Tile map editing with MSPaint pencil-like tool
* Tile map resizing

#### Notes

**BUG:** Before opening/saving/loading/editing anything, please ensure your current working directory is correct. If your tilemap file contains a tileset path as "../image/tileset.png", this file must exist, otherwise the application will crash. This will be fixed in a future update with TEdit project files.

#### Controls:

**Keys:**
* Escape -> Exit the application
* J -> add column to map's left side
* L -> add column to map's right side
* I -> add row to map's top side
* K -> add row to map's bottom side
* SHIFT+J -> remove column from map's left side
* SHIFT+L -> remove column from map's right side
* SHIFT+I -> remove row from map's top side
* SHIFT+K -> remove row from map's bottom side
* CTRL+N -> new tile map (not implemented yet)
* CTRL+S -> save current tile map
* CTRL+O -> open tile map

**Mouse:**
* Drag middle mouse button on tile map view -> pan
* Drag middle mouse button on tile set view -> pan
* Scroll wheel on tile map view -> zoom
* Scroll wheel on tile set view -> zoom
* Click on tile set view -> select hovered tile
* Click on tile map view -> set hovered tile to tile selected in tile set view

![Demo Image](./DEMO_IMAGE.png)

#### Planned:
* Custom properties (key/value pairs) attached to tiles
* Expand a 2D scene with 3D models (such as in the game [Realm of the Mad God](https://www.realmofthemadgod.com/))
* TypeScript library to efficiently load the maps in HTML5 canvas based games

## Build & Run
Uses [`xmake`](https://xmake.io/). Please [install](https://xmake.io/#/getting_started?id=installation) it first.

`ImGUI` is brought in as a submodule, so please clone the repository recursively.

```
$ git clone --recurse-submodules -j8 git://github.com/jprochazk/tedit.git
```

The project can be built in a number of ways. The simplest is running XMake directly:

```
$ xmake
$ xmake run
```

You can also [generate project files](https://xmake.io/#/plugin/builtin_plugins?id=generate-ide-project-files) for GNU Make, Visual Studio, and others:
```
$ xmake project -k makefile
$ make -j8
```

**NOTE:** Currently, only Windows is supported, due to the complexity of GUI environments on Linux and the fact that I don't have access to a Mac. In theory, this application should work on all of these platforms, but I doubt it will build without some platform-specific changes. If you try to build this and fail, please let me know by creating an [issue](https://github.com/jprochazk/tedit/issues).

### Why XMake?

Because for my purposes, this is the best option in terms of maintainability, flexibility, and effiency:

1. The `xmake` file is in written [Lua](http://www.lua.org/), which is a simple and readable, yet powerful scripting language.
2. `xmake` builds projects [as fast](https://xmake.io/#/getting_started?id=build-as-fast-as-ninja) as the [`ninja`](https://ninja-build.org/) build system. I've never had such easy, fast, and reproducible builds.
3. It's also a package manager. It can acquire dependencies from the it's own [package repository](https://github.com/xmake-io/xmake-repo), or third-party repositories, such as from [conan](https://conan.io/center/), [vcpkg](https://github.com/microsoft/vcpkg/tree/master/ports), among others. All of these can be included into the same project without any fuss.
4. It is well maintained, active, and popular, with more than 3k stars on [GitHub](https://github.com/xmake-io/xmake).
