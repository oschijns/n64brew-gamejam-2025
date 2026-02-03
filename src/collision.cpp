#include <t3d/t3d.h>
#include <t3d/t3dmath.h>
#include "collision.hpp"
#include "math/vec3.hpp"
#include "types.hpp"

#define TRUNC(x) static_cast<int16_t>(std::floor(x))

SphereCollider::SphereCollider(jam::Vec3 position, float radius) : mPosition(position), mRadius(radius) {
  mDebugVerts = (T3DVertPacked*)malloc_uncached(sizeof(T3DVertPacked) * 16);
}

// radius check for 2 spheres
bool SphereCollider::Collide(const SphereCollider& other)
{
  auto distance = jam::Vec3::distance(mPosition, other.mPosition);
  debugf("%.3f <=> (%.3f +- %.3f)\n", distance, mRadius, other.mRadius);
  return distance < mRadius + other.mRadius;
}

void SphereCollider::DebugDraw()
{
  // We'll define some fidelity of trianges
  // and then just draw against the index buffer
  constexpr int fidelity = 7;
  auto t = static_cast<T3DVec3>(mPosition);
  T3DVec3 coords[fidelity + 1] {
    t,
    t + (T3DVec3){-mRadius, 0, 0},
    t + (T3DVec3){0, -mRadius, 0},
    t + (T3DVec3){0, 0,-mRadius},
    t + (T3DVec3){mRadius, 0, 0},
    t + (T3DVec3){0, mRadius, 0},
    t + (T3DVec3){0, 0,mRadius},
    t
  };

  
  // Packed verts hold 2 at once
  T3DVec3 one {{0, 0, 1}};
  uint16_t norm = t3d_vert_pack_normal(&one); // normals are packed in a 5.6.5 format
  for (int i = 0; i < fidelity; i += 2)
  {
    T3DVec3 coordA = coords[i];
    T3DVec3 coordB = coords[i+1];
    mDebugVerts[static_cast<int32_t>(i / 2)] = (T3DVertPacked) {
      .posA = { TRUNC(coordA.x), TRUNC(coordA.y), TRUNC(coordA.z)},
      .normA = norm,
      .posB = { TRUNC(coordB.x), TRUNC(coordB.y), TRUNC(coordB.z)},
      .normB = norm,
      .rgbaA = 0xFF0000'FF,
      .rgbaB = 0xFF0000'FF,
    };
  }

  rdpq_mode_combiner(RDPQ_COMBINER_SHADE);
  t3d_state_set_drawflags((T3DDrawFlags)(T3D_FLAG_SHADED | T3D_FLAG_DEPTH));

  // Bezier points are already in world space
  // Called after camera setup
  t3d_vert_load(mDebugVerts, 0, fidelity);
  t3d_tri_draw(0, 1, 2);
  t3d_tri_draw(0, 3, 2);
  t3d_tri_draw(0, 4, 2);
  t3d_tri_draw(0, 6, 2);

  t3d_tri_draw(0, 1, 5);
  t3d_tri_draw(0, 3, 5);
  t3d_tri_draw(0, 4, 5);
  t3d_tri_draw(0, 6, 5);

  t3d_tri_sync(); // after each batch of triangles, a sync is needed
  // technically, you only need a sync before any new 't3d_vert_load', rdpq call, or after the last triangle
  // for safety, just call it after you are done with all triangles after a load
}

void SphereCollider::Update(const GameStateBook& gameState)
{
  const gamestate_page_t& current = gameState.back();
  if (current.phase == Phase::GARAGE)
  {
    return;
  }
  mPosition = current.lastPoint;
}
