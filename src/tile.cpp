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

    auto row = tile_id / perRow;
    auto col = tile_id % perRow;
    auto minU = row * singeTileUV.x;
    auto minV = col * singeTileUV.y;
    auto maxU = minU + singeTileUV.x;
    auto maxV = minV + singeTileUV.y;
    return { minU, minV, maxU, maxV };
}

namespace tile {

uint8_t
TileId(Tile tile)
{
    return tile & 0b00011111;
}

uint8_t
TileSetId(Tile tile)
{
    return (tile & 0b11100000) >> 5;
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
{}

TileMap::TileMap(const std::string& name, uint32_t columns, uint32_t rows, uint32_t tileSize)
  : name_(name)
  , columns_(columns)
  , rows_(rows)
  , tileSize_(tileSize)
  , tiles_(columns * rows)
  , tileSets_(TileSetId(std::numeric_limits<uint8_t>::max()))
{}

void
TileMap::add(TileSet* tileset)
{
    this->tileSets_.push_back(tileset);
}
void
TileMap::remove(TileSet* tileset)
{
    for (auto it = this->tileSets_.begin(); it != this->tileSets_.end(); ++it) {
        if ((*it) == tileset) {
            this->tileSets_.erase(it);
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

TileSet*
TileSet::Load(const std::string& path)
{
    // NOTE: thread unsafe
    static std::unordered_map<std::string, TileSet> gTileSetCache;

    if (auto it = gTileSetCache.find(path); it != gTileSetCache.end()) {
        return &(it->second);
    }
    auto it = gTileSetCache.emplace(path, TileSet(path));
    return &(it.first->second);
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
        std::cerr << "[ERROR]: " << ex.what() << std::endl;
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
        std::vector<uint8_t> tiles = json["tiles"];
        std::vector<std::string> tileSetSources = json["tileSets"];

        std::vector<TileSet*> tileSets;
        tileSets.reserve(tileSetSources.size());
        for (const auto& source : tileSetSources) {
            tileSets.emplace_back(TileSet::Load(source));
        }

        TileMap out;
        out.name_ = name;
        out.columns_ = columns;
        out.rows_ = rows;
        out.tileSize_ = tileSize;
        out.tiles_ = std::move(tiles);
        out.tileSets_ = std::move(tileSets);
        std::cout << out.tileSets_.size() << std::endl;
        return out;
    } catch (std::exception& ex) {
        spdlog::error("Error while loading tile map {}, {}", path, ex.what());
        return {};
    }
}

} // namespace tile