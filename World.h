//
// Created by jacob on 2025/9/10.
//

#ifndef WORLD_TEMP__WORLD_H_
#define WORLD_TEMP__WORLD_H_

#include "Chunk.h"
#include <cstdint>
#include <iostream>
#include <unordered_map>

class World {
 public:
  explicit World(uint64_t seed) : seed(seed){}
  void print_seed(){
    std::cout<<seed<<std::endl;
  }


 private:
  std::unordered_map<uint64_t, Chunk> _chunk_map;
  uint64_t seed;


};

#endif//WORLD_TEMP__WORLD_H_
