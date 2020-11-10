#include "pch.h"
#include "tile.hpp"

glm::vec4
calculate_tile_uv(tile::Tile tile, int tileSize, int atlasWidth, int altasHeight)
{
    auto tile_id = tile::TileId(tile);

    // tiles per row
    auto perRow = atlasWidth / tileSize;
    // the width/height of a single tile
    // width -> (0, width) mapped to (0, 1)
    // height -> (0, height) mapped to (0, 1)
    auto singeTileUV = glm::vec2{ tileSize / (float)atlasWidth, tileSize / (float)altasHeight };

    auto col = tile_id / perRow;
    auto row = tile_id % perRow;
    auto minU = col * singeTileUV.x;
    auto minV = row * singeTileUV.y;
    auto maxU = singeTileUV.x;
    auto maxV = singeTileUV.y;
    return { minU, minV, maxU, maxV };
}

namespace tile {

/*
// clang-format off

    A tile is:

    [ 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 ]
    [      TILE   ID      |    TILESET ID    ]

    * 10 bits for tile    id -> 1024 tiles    per tileset
    * 6  bits for tileset id -> 64   tilesets per tilemap

    This is entirely arbitrary, and may spontaneously change

// clang-format on
*/

uint16_t
TileId(const Tile& tile)
{
    return (tile & 0b1111111111111111Ui16);
}
uint16_t
TileSetId(const Tile& tile)
{
    return (tile & 0b1111110000000000Ui16) >> 10;
}
void
TileId(Tile& tile, uint16_t value)
{
    tile |= value;
}
void
TileSetId(Tile& tile, uint16_t value)
{
    tile |= (value << 10);
}

TileSet::TileSet(const std::string& source)
  : source_(source)
  , atlas_(source, GL_TEXTURE_2D)
{}

std::string
TileSet::source() const
{
    return this->source_;
}
const Image&
TileSet::atlas() const
{
    return this->atlas_;
}

TileMap::TileMap()
  : name_()
  , columns_()
  , rows_()
  , tileSize_()
  , tiles_()
  , tileSets_()
  , tileSetPaths_()
{}

TileMap::TileMap(const std::string& name, uint32_t columns, uint32_t rows, uint32_t tileSize)
  : name_(name)
  , columns_(columns)
  , rows_(rows)
  , tileSize_(tileSize)
  , tiles_(columns * rows)
  , tileSets_((size_t)2 ^ 6)
  , tileSetPaths_((size_t)2 ^ 6)
{}

void
TileMap::add(TileSet* tileset)
{
    this->tileSets_.push_back(tileset);
    this->tileSetPaths_.push_back(tileset->source());
}
void
TileMap::remove(TileSet* tileset)
{
    for (size_t i = 0; i < this->tileSets_.size(); ++i) {
        if (this->tileSets_[i] == tileset) {
            this->tileSets_.erase(this->tileSets_.begin() + i);
            this->tileSetPaths_.erase(this->tileSetPaths_.begin() + i);
            return;
        }
    }
}

Tile
TileMap::get(uint32_t x, uint32_t y)
{
    return this->tiles_[x + y * this->columns_];
}
void
TileMap::set(uint32_t x, uint32_t y, Tile value)
{
    this->tiles_[x + y * this->columns_] = value;
}
glm::vec4
TileMap::uv(Tile tile) const
{
    auto& atlas = this->tileSets_[TileSetId(tile)]->atlas();
    return calculate_tile_uv(tile, this->tileSize_, atlas.width(), atlas.height());
}
const TileSet*
TileMap::tileset(Tile tile) const
{
    return this->tileSets_[TileSetId(tile)];
}
TileSet*
TileMap::tileset(Tile tile)
{
    return this->tileSets_[TileSetId(tile)];
}

std::string
TileMap::name() const
{
    return this->name_;
}
uint32_t
TileMap::columns() const
{
    return this->columns_;
}
uint32_t
TileMap::rows() const
{
    return this->rows_;
}
uint32_t
TileMap::tileSize() const
{
    return this->tileSize_;
}
std::vector<TileSet*>&
TileMap::tilesets()
{
    return this->tileSets_;
}
std::vector<std::string>&
TileMap::tilesetPaths()
{
    return this->tileSetPaths_;
}

std::vector<Tile>::iterator
TileMap::begin()
{
    return this->tiles_.begin();
}
std::vector<Tile>::iterator
TileMap::end()
{
    return this->tiles_.end();
}
size_t
TileMap::size()
{
    return this->tiles_.size();
}

static std::unordered_map<std::string, std::unique_ptr<TileSet>> tileSetCache;
// NOTE: thread unsafe
TileSet*
TileSet::Load(const std::string& path)
{
    if (auto& it = tileSetCache.find(path); it == tileSetCache.end()) {
        tileSetCache.emplace(path, std::move(std::make_unique<TileSet>(path)));
        it = tileSetCache.find(path);
    }
    return tileSetCache[path].get();
}

void
TileMap::Save(TileMap& tm, const std::string& path)
{
    try {
        std::vector<std::string> tileSetSources;
        tileSetSources.reserve(tm.tileSets_.size());
        for (auto& ts : tm.tileSets_) {
            tileSetSources.emplace_back(ts->source());
        }

        // clang-format off
        json out = {};
        out["name"] = tm.name_;
        out["columns"] = tm.columns_;
        out["rows"] = tm.rows_;
        out["tileSize"] = tm.tileSize_;
        // TODO: layers
        out["tiles"] = tm.tiles_;
        out["tileSets"] = tileSetSources;
        // clang-format on

        std::ofstream file(path);
        file << out;
    } catch (std::exception& ex) {
        spdlog::error("Error while saving tile map to {}: {}", path, ex.what());
    }
}

std::optional<TileMap>
TileMap::Load(const std::string& path)
{
    try {
        std::ifstream file(path);
        json json = json::parse(file);

        std::string name = json["name"];
        uint32_t columns = json["columns"];
        uint32_t rows = json["rows"];
        uint32_t tileSize = json["tileSize"];
        // TODO: layers
        std::vector<Tile> tiles = json["tiles"];
        std::vector<std::string> tileSetSources = json["tileSets"];

        std::vector<TileSet*> tileSets;
        std::vector<std::string> tileSetPaths;
        tileSets.reserve(columns * rows);
        tileSetPaths.reserve(columns * rows);
        for (const auto& source : tileSetSources) {
            tileSets.emplace_back(TileSet::Load(source));
            tileSetPaths.emplace_back(std::move(source));
        }

        TileMap out;
        out.name_ = name;
        out.columns_ = columns;
        out.rows_ = rows;
        out.tileSize_ = tileSize;
        out.tiles_ = std::move(tiles);
        out.tileSets_ = std::move(tileSets);
        assert(out.tileSets_.capacity() == columns * rows);
        out.tileSetPaths_ = std::move(tileSetPaths);
        assert(out.tileSetPaths_.capacity() == columns * rows);
        return out;
    } catch (std::exception& ex) {
        spdlog::error("Error while loading tile map {}, {}", path, ex.what());
        return {};
    }
}

} // namespace tile