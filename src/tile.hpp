#ifndef TEDIT_TILE_
#define TEDIT_TILE_

#include "pch.h"
#include "gfx/image.hpp"

namespace tile {

using Tile = uint8_t;

inline uint8_t
id(Tile tile)
{
    return tile & 0b11100000;
}
inline uint8_t
tileset(Tile tile)
{
    return tile & 0b00011111;
}

class TileSet
{
public:
    TileSet(const std::string& uri, int tileSize);

    glm::vec4 uv(Tile tile) const;

    int getTileSize() const;
    Image* getImage();
    const Image* getImage() const;
    std::string getName() const;

private:
    std::string name;
    Image image;
    int imgW;
    int imgH;
    int perRow;
    int tileSize;
    glm::vec2 uvPerTile;
}; // struct TileSet

class TileMap
{
public:
    TileMap(const std::string& name, int cols, int rows, int tsize);

    void add(std::shared_ptr<TileSet> tileset);
    void remove(std::shared_ptr<TileSet> tileset);

    std::vector<Tile>::iterator begin();
    std::vector<Tile>::iterator end();
    size_t size() const;

    Tile& operator()(int x, int y);
    Tile operator()(int x, int y) const;
    void operator()(int x, int y, const Tile& value);

    std::string getName() const;

private:
    std::string name;
    // TODO: make this not shared_ptr
    std::unordered_map<std::string, std::shared_ptr<TileSet>> tilesets;
    // TODO: layers
    std::vector<Tile> tiles;
    int cols;
    int rows;
    int tsize;
}; // struct TileMap

void SaveMap(TileMap& tm, const std::string& path);
TileMap OpenMap(const std::string& path);

} // namespace tile

#endif // TEDIT_TILE_
