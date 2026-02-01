#pragma once
#include "math/vec3.hpp"
#include "types.hpp"
#include <t3d/t3d.h>
class SphereCollider {
  public:
    SphereCollider(jam::Vec3 position, float radius);
    bool Collide(const SphereCollider& other);
    void DebugDraw();
    void Update(const GameStateBook& gameState);
  private:
    jam::Vec3 mPosition;
    float mRadius;
    T3DVertPacked* mDebugVerts;
};
