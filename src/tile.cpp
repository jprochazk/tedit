#include "pch.h"
#include "tile.hpp"

namespace tile {

std::atomic_size_t GID;

TileSet::TileSet(const std::string& uri, int tileSize)
  : name(uri)
  , image(uri, GL_TEXTURE_2D)
  , imgW(this->image.getWidth())
  , imgH(this->image.getHeight())
  , perRow(this->imgW / tileSize)
  , tileSize(tileSize)
  , uvPerTile{ tileSize / (float)imgW, tileSize / (float)imgH }
{
    spdlog::info("New TileSet: {}, img w{} h{}, ts{}", uri, imgW, imgH, tileSize);
}

std::string
TileSet::getName() const
{
    return this->name;
}

glm::vec4
TileSet::uv(Tile tile) const
{
    auto tile_id = id(tile);

    auto row = tile_id / this->perRow;
    auto col = tile_id % this->perRow;
    auto minU = row * this->uvPerTile.x;
    auto minV = col * this->uvPerTile.y;
    auto maxU = minU + this->uvPerTile.x;
    auto maxV = minV + this->uvPerTile.y;
    return { minU, minV, maxU, maxV };
}

int
TileSet::getTileSize() const
{
    return this->tileSize;
}

Image*
TileSet::getImage()
{
    return &this->image;
}

const Image*
TileSet::getImage() const
{
    return &this->image;
}

TileMap::TileMap(const std::string& name, int cols, int rows, int tsize)
  : name(name)
  , tiles(static_cast<size_t>(cols * rows))
  , cols(cols)
  , rows(rows)
  , tsize(tsize)
{}

void
TileMap::add(std::shared_ptr<TileSet> tileset)
{
    this->tilesets.emplace(tileset->getName(), tileset);
}

void
TileMap::remove(std::shared_ptr<TileSet> tileset)
{
    this->tilesets.erase(tileset->getName());
}

std::vector<Tile>::iterator
TileMap::begin()
{
    return this->tiles.begin();
}

std::vector<Tile>::iterator
TileMap::end()
{
    return this->tiles.end();
}

size_t
TileMap::size() const
{
    return this->tiles.size();
}

Tile&
TileMap::operator()(int x, int y)
{
    return tiles[x + y * this->cols];
}
Tile
TileMap::operator()(int x, int y) const
{
    return tiles[x + y * this->cols];
}

void
TileMap::operator()(int x, int y, const Tile& value)
{
    tiles[x + y * this->cols] = value;
}

std::string
TileMap::getName() const
{
    return this->name;
}

static thread_local simdjson::dom::parser parser;

void
SaveMap(TileMap& tm, const std::string& path)
{
    (void)tm;
    (void)path;
}

TileMap
OpenMap(const std::string& path)
{
    return TileMap(path, 32, 32, 32);
}

} // namespace tile