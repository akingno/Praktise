//
// Created by jacob on 2025/9/11.
//

#include "Character.h"


void Character::move() {
  std::random_device rd;
  std::mt19937  gen(rd());

  switch (direction(gen)){
    case 1:
      _loc.first +=2;
      break;
    case 2:
      _loc.first -=1;
      break;
    case 3:
      _loc.second +=1;
      break;
    case 4:
      _loc.second -=1;
      break;
  }

}
