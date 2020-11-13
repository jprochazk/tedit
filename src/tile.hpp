#include "pch.h"

#ifndef TEDIT_TILE_
#define TEDIT_TILE_

#include "gfx/image.hpp"

namespace tile {

/*

JSON representation:

Tile = number
TileSet = string

TileMap {
        name: string,

        // TODO: layers
        columns: number,
        rows: number,
        tileSize: number,

        tiles: Tile[]
        tileSets: TileSet[],
}

In-memory representation:

Tile = U32;
TileSet {
    string source
    Image atlas
}
TileMap {
    name: string

    tileSets: vector<TileSet>

    columns: U32
    rows: U32
    tileSize: U32

    tiles: vector<Tile>
}

*/

using Tile = uint16_t;
// Getter
uint16_t TileId(const Tile& tile);
// Getter
uint16_t TileSetId(const Tile& tile);
// Setter
void TileId(Tile& tile, uint16_t value);
// Setter
void TileSetId(Tile& tile, uint16_t value);

class TileSet
{
public:
    TileSet(const std::string& source);

    std::string source() const;
    const Image& atlas() const;

    static TileSet* Load(const std::string& path);

private:
    std::string source_;
    Image atlas_;
}; // class TileSet

class TileMap
{
public:
    TileMap();
    TileMap(const std::string& name, uint32_t columns, uint32_t rows, uint32_t tileSize);

    void add(TileSet* tileset);
    void remove(TileSet* tileset);

    Tile get(uint32_t x, uint32_t y);
    void set(uint32_t x, uint32_t y, Tile value);

    glm::vec4 uv(Tile tile) const;
    const TileSet* tileset(Tile tile) const;
    TileSet* tileset(Tile tile);

    std::string name() const;
    uint32_t columns() const;
    uint32_t rows() const;
    uint32_t tileSize() const;
    std::vector<TileSet*>& tilesets();
    std::vector<std::string>& tilesetPaths();

    std::vector<Tile>::iterator begin();
    std::vector<Tile>::iterator end();
    size_t size();

    static void Save(TileMap& tm, const std::string& path);
    static std::unique_ptr<TileMap> Load(const std::string& path);

private:
    std::string name_;
    uint32_t columns_;
    uint32_t rows_;
    uint32_t tileSize_;
    std::vector<Tile> tiles_;
    std::vector<TileSet*> tileSets_;
    std::vector<std::string> tileSetPaths_;
}; // class TileMap

} // namespace tile

#endif // TEDIT_TILE_
