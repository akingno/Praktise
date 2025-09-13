//
// Created by jacob on 2025/9/12.
//

#ifndef WORLD_TEMP__RANDOM_H_
#define WORLD_TEMP__RANDOM_H_

#include <random>
#include <cstdint>
#include <array>
#include <atomic>

namespace Random {

// 基本分布
double rand01();                 // [0,1)
bool   bernoulli(double p);      // 以概率 p 返回 true
int    randint(int lo, int hi);  // 闭区间 [lo, hi]

// —— 可选：为不同子系统提供“子流标签”，保证可复现
void   set_stream(uint64_t tag); // 例如 0=默认, 1=Agent, 2=Loot...

}

#endif//WORLD_TEMP__RANDOM_H_
