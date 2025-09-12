//
// Created by jacob on 2025/9/10.
//

#ifndef WORLD_TEMP__CAMERA_H_
#define WORLD_TEMP__CAMERA_H_

#include <utility>
class Camera {

 public:

  Camera(){
    _pos = {0.0, 0.0};
  }

  void onUpdate(std::pair<float, float> new_pos){
    _pos = new_pos;
  }

  [[nodiscard]] std::pair<float, float> getPos() const{
    return _pos;
  }

 private:
  std::pair<float, float> _pos;
};

#endif//WORLD_TEMP__CAMERA_H_
