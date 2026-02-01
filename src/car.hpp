#pragma once
#include "joypad.h"
#include "types.hpp"
#include "collision.hpp"
#include "math/bezier.hpp"
#include "math/vec3.hpp"
#include <t3d/t3dmath.h>
#include <t3d/t3dmodel.h>
class Car {
private:
  float speedMagnitude = .05f;
  T3DMat4FP *loc[2]{
    (T3DMat4FP *)malloc_uncached(sizeof(T3DMat4FP)),
    (T3DMat4FP *)malloc_uncached(sizeof(T3DMat4FP))
  };
  T3DVec3 corrected {{0.f, 0.f, 0.f}};
  jam::Vec3 currentTrackPoint{0, 0, 0};
  jam::Vec3 currentLocal{0, 0, 0};
  jam::Vec3 lastDirection{0.f, 0.f, 0.f};
  jam::BezierTrack::Iterator trackIter = nullptr;
  T3DQuat currentRotation;
  T3DQuat spin;

  T3DModel* carModel;
  unsigned short lap = 0;
  color_t* garageColors;

public:
  Car(jam::BezierTrack& track);
  void SetColors(int numColors, color_t* colors);
  void Update(GameStateBook& gameStateHistory, joypad_buttons_t buttons);
  void Render(T3DModelState& state);
};

