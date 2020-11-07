# tedit

## Tile Map Editor written in C++

## Features

#### Planned:
* Tile sets from images
* Tile maps
* Custom properties (key/value pairs) attached to tiles
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

1. The `xmake` file is in written [Lua](http://www.lua.org/), which is a simple, readable, yet powerful scripting language.
2. Builds your project [as fast](https://xmake.io/#/getting_started?id=build-as-fast-as-ninja) as the [`ninja`](https://ninja-build.org/) build system
3. It's also a [package manager](https://github.com/xmake-io/xmake-repo).
4. It is well maintained, active and popular, with more than 3k stars on [GitHub](https://github.com/xmake-io/xmake).
