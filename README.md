# tedit

This repository hosts a Tile-based Map Editor written in C++. 

It uses:
* [GLFW](https://www.glfw.org/) for window and input handling
* [OpenGL](https://www.opengl.org/) for rendering
* [ImGUI](https://github.com/ocornut/imgui) for UI rendering
* [simdjson](https://github.com/simdjson/simdjson) for json serialization/deserialization
* [STB](https://github.com/nothings/stb) for image loading
* [GLM](https://glm.g-truc.net/0.9.9/index.html) for vector mathematics

## Features

#### Planned:
* Tile sets from images
* Tile maps
* Custom properties (key/value pairs) attached to tiles
* Expand a 2D scene with 3D models (such as in the game [Realm of the Mad God](https://www.realmofthemadgod.com/))
* Import from and export to (optionally) optimized JSON
* TypeScript library to efficiently load the maps in HTML5 canvas based games

## Build & Run

Uses [`xmake`](https://xmake.io/). Please [install](https://xmake.io/#/getting_started?id=installation) it first.

```
$ xmake
$ xmake run
```

### Why XMake?

Because for my purposes, this is the best option in terms of maintainability, flexibility and effiency:

1. The `xmake` file is in written [Lua](http://www.lua.org/), which is a simple and readable, yet powerful scripting language.
2. Builds your project [as fast](https://xmake.io/#/getting_started?id=build-as-fast-as-ninja) as the [`ninja`](https://ninja-build.org/) build system. I've never had such easy, fast, and reproducible builds.
3. It's also a package manager. It can acquire dependencies from the it's own [package repository](https://github.com/xmake-io/xmake-repo), or third-party repositories, such as from [conan](https://conan.io/center/), [vcpkg](https://github.com/microsoft/vcpkg/tree/master/ports), among others. All of these can be included into the same project without any fuss.
4. It is well maintained, active, and popular, with more than 3k stars on [GitHub](https://github.com/xmake-io/xmake), it has daily commits and fast response times (which I've personally experienced).
