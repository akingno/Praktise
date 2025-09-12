//
// Created by jacob on 2025/9/11.
//

#ifndef WORLD_TEMP__CHARACTER_H_
#define WORLD_TEMP__CHARACTER_H_

#include <random>
#include <utility>
class Character {

 public:
  Character() :direction(1, 4), _loc(1.0f,1.0f) {
  }

  [[nodiscard]] std::pair<float, float> getLoc() const{return _loc; }
  void move();

 private:
  std::pair<float, float> _loc;
  std::uniform_int_distribution<> direction;
};

#endif//WORLD_TEMP__CHARACTER_H_
