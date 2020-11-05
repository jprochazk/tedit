
type Tile* = distinct uint8

proc tilesetId*(tile: Tile): uint8 = uint8(tile) and 0b11100000'u8
proc tileId*(tile: Tile): uint8 = uint8(tile) and 0b00011111'u8

type TileMap* = object
  tiles: UncheckedArray[Tile]
  width: int
  height: int

proc get(tm: TileMap, x: int, y: int): Tile = tm.tiles[y * tm.width + x]
proc set(tm: var TileMap, x: int, y: int, tile: Tile) = tm.tiles[y * tm.width + x] = tile
