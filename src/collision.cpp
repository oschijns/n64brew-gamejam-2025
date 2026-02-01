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
  return (mRadius - other.mRadius) <= distance && (mRadius + other.mRadius) > distance;
}

void SphereCollider::DebugDraw()
{
  // We'll define some fidelity of trianges
  // and then just draw against the index buffer
  constexpr int fidelity = 7;
  jam::Vec3 coords[fidelity + 1] {
    {mPosition.coords[0], mPosition.coords[1], mPosition.coords[2]},
    
    {mPosition.coords[0] - mRadius, mPosition.coords[1], mPosition.coords[2]},
    {mPosition.coords[0], mPosition.coords[1] - mRadius, mPosition.coords[2]},
    {mPosition.coords[0], mPosition.coords[1], mPosition.coords[2] - mRadius},

    {mPosition.coords[0] + mRadius, mPosition.coords[1], mPosition.coords[2]},
    {mPosition.coords[0], mPosition.coords[1] + mRadius, mPosition.coords[2]},
    {mPosition.coords[0], mPosition.coords[1], mPosition.coords[2] + mRadius},

    {mPosition.coords[0], mPosition.coords[1], mPosition.coords[2]} // Extra for padding
  };

  
  // Packed verts hold 2 at once
  T3DVec3 one {{0, 0, 1}};
  uint16_t norm = t3d_vert_pack_normal(&one); // normals are packed in a 5.6.5 format
  for (int i = 0; i < fidelity; i += 2)
  {
    T3DVec3 coordA = static_cast<T3DVec3>(coords[i]);
    T3DVec3 coordB = static_cast<T3DVec3>(coords[i]);
    mDebugVerts[i] = (T3DVertPacked) {
      .posA = { TRUNC(coordA.x), TRUNC(coordA.y), TRUNC(coordA.z)},
      .normA = norm,
      .posB = { TRUNC(coordB.x), TRUNC(coordB.y), TRUNC(coordB.z)},
      .normB = norm,
      .rgbaA = 0xFF0000'FF,
      .rgbaB = 0xFF0000'FF,
    };
    debugf("Coords (%.2f, %d, %d, %d)\n", coordA.x, mDebugVerts[i].posA[0], mDebugVerts[i].posA[1], mDebugVerts[i].posA[2]);
  }

  rdpq_mode_combiner(RDPQ_COMBINER_SHADE);
  t3d_state_set_drawflags((T3DDrawFlags)(T3D_FLAG_SHADED | T3D_FLAG_DEPTH));

  // Bezier points are already in world space
  // Called after camera setup
  t3d_vert_load(mDebugVerts, 0, fidelity);
  t3d_tri_draw(0, 1, 2);
  // t3d_tri_draw(0, 2, 3);
  // t3d_tri_draw(0, 3, 4);
  // t3d_tri_draw(0, 4, 5);
  // t3d_tri_draw(0, 5, 6);
  // t3d_tri_draw(0, 6, 7);
  // t3d_tri_draw(0, 7, 1);

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
