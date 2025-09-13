//
// Created by jacob on 2025/9/12.
//

#include "Random.h"


namespace {
// 轻量 SplitMix64 做播种扩展
static inline uint64_t splitmix64(uint64_t x) {
  x += 0x9e3779b97f4a7c15ULL;
  x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
  x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
  return x ^ (x >> 31);
}
struct RNGState {
  uint64_t base = 0;
};
thread_local RNGState  gstate;
thread_local std::mt19937_64 grng;
}

namespace Random{
void set_stream(uint64_t tag) {
  grng.seed(splitmix64(gstate.base ^ (tag * 0x9e3779b97f4a7c15ULL)));
}
double rand01() {
  static thread_local std::uniform_real_distribution<double> dist(0.0, 1.0);
  return dist(grng);
}
bool bernoulli(double p) {
  std::bernoulli_distribution d(p);
  return d(grng);
}
int randint(int lo, int hi) {
  std::uniform_int_distribution<int> d(lo, hi);
  return d(grng);
}
}