//
// Created by jacob on 2025/9/10.
//

#ifndef WORLD_TEMP__CHUNK_H_
#define WORLD_TEMP__CHUNK_H_

#include "Block.h"
#include <array>
#include <cstdint>
#include "Noise.h"

constexpr int CHUNK_SIZE = 64;


class Chunk {
 public:
  Chunk(uint64_t seed, int cx, int cy) : _coor_chunk{cx, cy}, _seed(seed) {
    Fbm2D fbm(seed);         // 用世界 seed；也可在 World 里共享一个实例

    /*
     *  海岛风格：octaves=5, lacunarity=2.0, gain=0.5, base_freq≈1/100
        高山大陆：octaves=6, lacunarity=2.2, gain=0.55, base_freq≈1/80
        平缓丘陵：octaves=4, lacunarity=1.9, gain=0.4, base_freq≈1/120
     *
     * */
    fbm.octaves = 5;         // 层数:可调：4~6, 小: 大块大陆海洋, 大: 山脉河谷
    fbm.lacunarity = 2.0;     //间隙, 缩放, <2则细节过度更平滑
    fbm.gain = 0.5;           //控制高频的贡献大小, 小则平滑, 大则地形多样丰富

    // 基础频率：决定地形“尺度”（越小越平缓、特征更大）
    // 你可以把 0.01 调为 1.0 / 80 或 1.0 / 96 等
    const double base_freq = 1.0 / 80.0;

    // 平铺数组：行主序 idx = j*CHUNK_SIZE + i
    for (int j = 0; j < CHUNK_SIZE; ++j) {
      for (int i = 0; i < CHUNK_SIZE; ++i) {
        int wx = cx * CHUNK_SIZE + i;
        int wy = cy * CHUNK_SIZE + j;
        double h = fbm.fbm(wx * base_freq, wy * base_freq);
        _blocks[j * CHUNK_SIZE + i]._tile_type = Block::height_to_tile(h);
      }
    }
  }

  const Block& at(int lx, int ly) const { return _blocks[ly * CHUNK_SIZE + lx]; }
  Block&       at(int lx, int ly)       { return _blocks[ly * CHUNK_SIZE + lx]; }

 private:
  std::pair<int,int> _coor_chunk;
  uint64_t _seed;
  std::array<Block, CHUNK_SIZE*CHUNK_SIZE> _blocks;
};

#endif//WORLD_TEMP__CHUNK_H_
