//
// Created by jacob on 2025/9/11.
//

#ifndef WORLD_TEMP__CHARACTER_H_
#define WORLD_TEMP__CHARACTER_H_

#include "Random.h"
#include <random>
#include <utility>

enum class Dir{
  Right = 0,
  Left = 1,
  Down = 2,
  Up = 3
};

struct DirWeights{
 const int right = 2, left = 1, down = 2, up = 2;
 public:
  [[nodiscard]] int sum() const{
    return right + left + down + up;
  }
  [[nodiscard]] Dir random_dir_weights() const{
    double r = Random::rand01() * sum();
    if((r -= right) < 0 ) return Dir::Right;
    if((r -= left) < 0 ) return Dir::Left;
    if((r -= down) < 0 ) return Dir::Down;
    return Dir::Up;
  }
};

static std::pair<int, int> dir_vec(Dir d){
  switch (d) {
    case Dir::Right: return {+1, 0};
    case Dir::Left: return  {-1, 0};
    case Dir::Down: return  {0,+1};
    case Dir::Up:   return  {0, -1};
  }
  return {0, 0};
}

static Dir opposite(Dir d){
  switch (d) {
    case Dir::Right: return Dir::Left;
    case Dir::Left: return  Dir::Right;
    case Dir::Down: return  Dir::Up;
    case Dir::Up:   return  Dir::Down;
  }
  return d;
}




class Character {

 public:
  static Dir Int2Dir(int num) {
    switch (num) {
      case 0:
        return Dir::Right;
      case 1:
        return Dir::Left;
      case 2:
        return Dir::Down;
      case 3:
        return Dir::Up;
      default:
        return Dir::Right;
    }
  }
  Character() : _loc(1.0f,1.0f) {
    int dir = Random::randint(0,3);
    last_dir_ = Int2Dir(dir);
  }

  [[nodiscard]] std::pair<float, float> getLoc() const{return _loc; }

  template<class IsPassable>
  bool tryMove(IsPassable&& is_passable, double keep_prob=0.55){
    Dir order[4];

    order[0] = pick_biased_dir(last_dir_, keep_prob);
    order[1] = opposite(order[0]);

    int oi=2;
    for (Dir d : {Dir::Right, Dir::Left, Dir::Down, Dir::Up}) {
      if (d!=order[0] && d!=order[1]) order[oi++]=d;
    }

    for (Dir d : order) {
      auto [dx,dy] = dir_vec(d);
      int nx = (int)_loc.first + dx;
      int ny = (int)_loc.second + dy;
      if (is_passable(nx, ny)) { _loc = {nx,ny}; last_dir_ = d; return true; }
    }
    return false; // 四个方向都不行
  }

 private:
  Dir pick_biased_dir(Dir last, double keep_prob=0.55) {
    if (Random::bernoulli(keep_prob)) return last;

    return weights.random_dir_weights();

  }

 private:
  std::pair<float, float> _loc;
  Dir last_dir_;
  DirWeights weights;
};

#endif//WORLD_TEMP__CHARACTER_H_
