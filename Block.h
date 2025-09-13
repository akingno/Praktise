//
// Created by jacob on 2025/9/10.
//

#ifndef WORLD_TEMP__BLOCK_H_
#define WORLD_TEMP__BLOCK_H_
#include <cstdint>
#include <utility>

enum class TileType{
  Grass = 0,
  Sea = 1,
  Rock = 2,
  Snow = 3
};

class Block {

 public:
  explicit Block(uint8_t tileType){
      _tile_type = tileType;
  };

  static inline char tile_glyph(TileType t) {
    switch (t) {
      case TileType::Sea:  return '~';
      case TileType::Rock: return '^';
      case TileType::Snow: return '*';
      case TileType::Grass:
      default:             return '.';
    }
  }

  static inline TileType Int2Tile(int8_t num){
    switch (num) {
      case 0:
        return TileType::Grass;
      case 1:
        return TileType::Sea;
      case 2:
        return TileType::Rock;
      case 3:
        return TileType::Snow;
      default:
        return TileType::Grass;
    }
  }

  static inline uint8_t height_to_tile(double h, double t_water = -0.12, double t_rock = 0.3, double t_snow = 0.55) {
    // h ∈ [-1,1]（大概），阈值可调
    if (h < t_water) return 1;   // 低处是水：~
    if (h > t_rock && h < t_snow)  return 2;  // 高处是山：^
    if (h > t_snow) return 3;
    return 0;                  // 中间是草：.
  }

  Block()= default;
  uint8_t _tile_type;

};

#endif//WORLD_TEMP__BLOCK_H_
