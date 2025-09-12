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
  Rock = 2
};

class Block {

 public:
  explicit Block(uint8_t tileType){
      _tile_type = tileType;
  };
  Block()= default;
  uint8_t _tile_type;

};

#endif//WORLD_TEMP__BLOCK_H_
