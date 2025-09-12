//
// Created by jacob on 2025/9/11.
//

#ifndef WORLD_TEMP__NOISE_H_
#define WORLD_TEMP__NOISE_H_


// noise.h
#pragma once
#include <algorithm>
#include <array>
#include <cstdint>
#include <random>

struct Perlin2D {
  std::array<int, 512> p; // permutation table (重复一份到512，便于索引)

  explicit Perlin2D(uint64_t seed) {
    std::array<int, 256> base{};
    for (int i = 0; i < 256; ++i) base[i] = i;
    std::mt19937_64 rng(seed);
    std::shuffle(base.begin(), base.end(), rng);
    for (int i = 0; i < 256; ++i) { p[i] = base[i]; p[256 + i] = base[i]; }
  }

  static inline double fade(double t) { return t*t*t*(t*(t*6 - 15) + 10); }
  static inline double lerp(double a, double b, double t) { return a + t*(b - a); }
  static inline double grad(int hash, double x, double y) {
    // 12 个方向中的一个
    int h = hash & 7; // 8方向已够用
    double u = (h < 4) ? x : y;
    double v = (h < 4) ? y : x;
    return ((h & 1) ? -u : u) + ((h & 2) ? -v : v);
  }

  // 2D Perlin，返回约在 [-1,1]
  double noise(double x, double y) const {
    int X = static_cast<int>(std::floor(x)) & 255;
    int Y = static_cast<int>(std::floor(y)) & 255;
    double xf = x - std::floor(x);
    double yf = y - std::floor(y);
    double u = fade(xf), v = fade(yf);

    int aa = p[p[X] + Y];
    int ab = p[p[X] + Y + 1];
    int ba = p[p[X + 1] + Y];
    int bb = p[p[X + 1] + Y + 1];

    double x1 = lerp(grad(aa, xf,     yf    ),
                     grad(ba, xf-1.0, yf    ), u);
    double x2 = lerp(grad(ab, xf,     yf-1.0),
                     grad(bb, xf-1.0, yf-1.0), u);
    return lerp(x1, x2, v); // [-1,1] 左右
  }
};

// 多倍频 Fbm 包装：octaves / lacunarity / gain
struct Fbm2D {
  Perlin2D perlin;
  int octaves = 5;
  double lacunarity = 2.0;   // 频率倍增
  double gain       = 0.5;   // 振幅衰减

  explicit Fbm2D(uint64_t seed) : perlin(seed) {}

  // 输入 x,y 用 "世界坐标 * 频率"（比如 0.01）
  double fbm(double x, double y) const {
    double amp = 1.0, freq = 1.0, sum = 0.0, norm = 0.0;
    for (int i = 0; i < octaves; ++i) {
      sum  += amp * perlin.noise(x * freq, y * freq);
      norm += amp;
      amp *= gain;
      freq *= lacunarity;
    }
    return sum / norm; // 约 [-1,1]
  }
};




#endif//WORLD_TEMP__NOISE_H_
