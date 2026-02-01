#include "colors.hpp"
#include "car.hpp"
#include "joypad.h"
#include "t3d/t3d.h"
#include "types.hpp"
#include "math/bezier.hpp"
#include <cmath>
#include <cstddef>
#include <t3d/t3dmodel.h>
#include <t3d/t3dmath.h>

void print_vec(const char* name, const jam::Vec3& vec, bool buffer = false)
{
  debugf("= %s: %.4f, %.4f, %.4f =", name, vec.x, vec.y, vec.z);
  if (buffer) debugf("\n");
}

Car::Car(jam::BezierTrack& track)
{
  trackIter = track.begin();
  auto tempIter = track.begin();
  ++tempIter;
  currentTrackPoint = (*trackIter).position;
  currentLocal = (*trackIter).position;
  lastDirection = (currentTrackPoint - currentLocal).normalize();

  carModel = t3d_model_load("rom://car.t3dm");
  t3d_quat_identity(spin);
  t3d_quat_identity(currentRotation);
}

void Car::Render(T3DModelState& state) {
  // it.object->material->setColorFlags &= 0b110;
  t3d_mat4fp_from_srt(loc[0], (float[3]){0.1f, 0.1f, 0.1f},
                            currentRotation.v, corrected.v);
  t3d_mat4fp_from_srt(loc[1], (float[3]){1.f, 1.f, 1.f},
                            spin.v, (float[3]){0.f, 0.f, 0.f});
  t3d_matrix_push(loc[0]);
  t3d_matrix_push(loc[1]);

  auto iter = t3d_model_iter_create(carModel, T3D_CHUNK_TYPE_OBJECT);
  int32_t i = 0;
  while (t3d_model_iter_next(&iter)) {
    iter.object->material->setColorFlags &= 0b110; 
    rdpq_set_prim_color(garageColors[i]);
    t3d_model_draw_material(iter.object->material, &state);
    t3d_model_draw_object(iter.object, NULL);
    state = t3d_model_state_create();
    i += 1;
  }
  t3d_matrix_pop(2);
}

void Car::SetColors(int numColors, color_t *colors) {
  assertf(numColors == numBlocks, "Too many colors have been passed in");
  garageColors = colors;
}

void Car::Update(GameStateBook &gameStateHistory, joypad_buttons_t buttons) {
  gamestate_page_t& current = gameStateHistory.back();

  if (lap == 3) {
    current.phase = Phase::GARAGE;
    lap = 0;
    // return early
    return;
  }
  jam::Vec3 newPosition = currentLocal;
  if (buttons.a)
  {
    // Have we passed the current track point in our current direction
    auto p = lastDirection * speedMagnitude; // @todo: This is currently ignoring tick delta
    newPosition = currentLocal + p;
    jam::Vec3 expectedDir = currentTrackPoint - currentLocal;
    jam::Vec3 headingDir = newPosition - currentLocal;
    // Model faces (0, 0, 1)
    float angleToTurn = std::acos(expectedDir.normal().dot({1.f, 0.f, 0.f}));
    t3d_quat_identity(currentRotation);
    t3d_quat_rotate_euler(currentRotation, (float[3]){0,1.f, 0.f}, angleToTurn);
    if (newPosition == currentTrackPoint)
    {
      // we can skip this one
      ++trackIter;
      // Have we passed the last segment?
      if (trackIter.end())
      {
        trackIter.reset();
        lap += 1;
      }
      currentTrackPoint = (*trackIter).position;
    }
    else
    {
      auto proj = headingDir.projected_onto(expectedDir);
      if (proj.mag() >= expectedDir.mag())
      {
        // We've passed this point
        ++trackIter;
        // Have we passed the last segment?
        if (trackIter.end())
        {
          trackIter.reset();
          lap += 1;
        }
        currentTrackPoint = (*trackIter).position;
      }
    }
    lastDirection = (currentTrackPoint - newPosition).normalize();
  }

  t3d_quat_identity(spin);
  float pi2 = 3.1415 / 2.f;
  if (buttons.l) {
    if (buttons.c_left || buttons.c_down) {
      // Rotate around Z
      t3d_quat_rotate_euler(spin, (float[3]){0.f, 0.f, 1.f}, pi2 * (buttons.c_up ? 1.f : -1.f));
    } else if (current.lastYInput != 0) {
      // Rotate around Y
      t3d_quat_rotate_euler(spin, (float[3]){0.f, 1.f, 0.f}, pi2 * current.lastYInput);
    } else if (current.lastXInput != 0) {
      // Rotate around X
      t3d_quat_rotate_euler(spin, (float[3]){1.f, 0.f, 0.f}, pi2 * current.lastXInput);
    }
  }
  jam::Vec3 normal(0, 0, 0);
  if (gameStateHistory.size() > 5 && !buttons.l) {
    float frac = 0.f;
    for (auto it = gameStateHistory.rbegin(); it != gameStateHistory.rbegin() + 5; it++) {
      frac += (0.2f * it->lastXInput);
    }
    normal = (*trackIter).data.normal * frac;
  }
  corrected = static_cast<T3DVec3>(newPosition + normal);
  currentLocal = newPosition;
  current.lastPoint = currentLocal;
}
