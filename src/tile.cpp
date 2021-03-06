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
    return (tile & static_cast<uint16_t>(0b0000001111111111));
}
uint16_t
TileSetId(const Tile& tile)
{
    return (tile & static_cast<uint16_t>(0b1111110000000000)) >> 10;
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
const gfx::Image&
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
  , tileSetIdSequence_()
  , tileSets_()
  , tileSetPaths_()
{}

TileMap::TileMap(const std::string& name, uint32_t columns, uint32_t rows, uint32_t tileSize)
  : name_(name)
  , columns_(columns)
  , rows_(rows)
  , tileSize_(tileSize)
  , tiles_()
  , tileSetIdSequence_(0)
  , tileSets_()
  , tileSetPaths_()
{
    this->tiles_.resize(columns * rows, Tile(-1));
    this->tileSets_.resize((2 << 5) - 1, nullptr);
    this->tileSetPaths_.reserve((2 << 5) - 1);
}

void
TileMap::resize(Direction direction, int count)
{
    auto old_columns = this->columns_;
    auto old_rows = this->rows_;
    auto old_size = this->columns_ * this->rows_;

    size_t new_columns = old_columns;
    size_t new_rows = old_rows;
    size_t new_size;
    if (direction == Direction::Left || direction == Direction::Right) {
        new_size = old_size + (old_rows * count);
        new_columns += count;
    } else {
        new_size = old_size + (old_columns * count);
        new_rows += count;
    }

    std::vector<Tile> new_tiles;
    new_tiles.resize(new_size, Tile(-1));
    for (size_t i = 0; i < this->tiles_.size(); ++i) {
        auto old_column = i / old_rows;
        auto old_row = i % old_rows;
        int new_column = direction == Direction::Left ? old_column + count : old_column;
        int new_row = direction == Direction::Bottom ? old_row + count : old_row;

        // only check for overflow if we're shrinking
        if (count < 0 && (new_column < 0 || new_column >= new_columns || new_row < 0 || new_row >= new_rows))
            continue;

        size_t new_pos = new_column + new_row * new_columns;
        size_t old_pos = old_column + old_row * old_columns;
        new_tiles.at(new_pos) = this->tiles_.at(old_pos);
    }

    this->columns_ = new_columns;
    this->rows_ = new_rows;
    this->tiles_ = std::move(new_tiles);
}

void
TileMap::add(TileSet* tileset)
{
    spdlog::info("Added TileSet {}@{} to TileMap {}", tileset->source(), (void*)tileset, this->name());
    this->tileSets_[tileSetIdSequence_++] = tileset;
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

const Tile&
TileMap::operator()(uint32_t x, uint32_t y) const
{
    return this->tiles_[x + y * columns_];
}
Tile&
TileMap::operator()(uint32_t x, uint32_t y)
{
    return this->tiles_[x + y * columns_];
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
    // TODO: return nullptr if we don't have this tileset
    auto id = TileSetId(tile);
    if (id < this->tileSets_.size())
        return this->tileSets_[TileSetId(tile)];
    else
        return nullptr;
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

static std::unordered_map<std::string, std::unique_ptr<TileSet>> tileSetCache;
// NOTE: thread unsafe
TileSet*
TileSet::Load(const std::string& path)
{
    if (auto it = tileSetCache.find(path); it == tileSetCache.end()) {
        tileSetCache.emplace(path, std::move(std::make_unique<TileSet>(path)));
    }
    return tileSetCache[path].get();
}

std::unique_ptr<TileMap>
TileMap::Create(const std::string& name, uint32_t columns, uint32_t rows)
{
    // TODO: variable tilesize
    return std::make_unique<TileMap>(name, columns, rows, 32);
}

void
TileMap::Save(TileMap& tm, const std::string& path)
{
    try {
        std::vector<std::string> tileSetSources;
        tileSetSources.reserve(tm.tileSets_.size());
        for (auto ts : tm.tileSets_) {
            if (ts != nullptr) {
                tileSetSources.emplace_back(ts->source());
            }
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

std::unique_ptr<TileMap>
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
        tileSets.resize((2 << 5) - 1, nullptr);
        tileSetPaths.reserve((2 << 5) - 1);
        for (size_t i = 0; i < tileSetSources.size(); ++i) {
            tileSets[i] = TileSet::Load(tileSetSources[i]);
            tileSetPaths.emplace_back(std::move(tileSetSources[i]));
        }

        auto out = std::make_unique<TileMap>();
        out->name_ = name;
        out->columns_ = columns;
        out->rows_ = rows;
        out->tileSize_ = tileSize;
        out->tiles_ = std::move(tiles);
        out->tileSets_ = std::move(tileSets);
        out->tileSetPaths_ = std::move(tileSetPaths);
        return std::move(out);
    } catch (std::exception& ex) {
        spdlog::error("Error while loading tile map {}, {}", path, ex.what());
        return nullptr;
    }
}

} // namespace tile