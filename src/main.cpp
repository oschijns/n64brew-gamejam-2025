/*
 * Entry point
 */

#include <cmath>   // floor, tan
#include <cstring> // memcpy
#include <iterator>
#include <libdragon.h>
#include <deque>
#include <t3d/t3d.h>
#include <t3d/t3dmodel.h>
#include <t3d/t3dmath.h>
#include <vector>

#include "graphics.h"
#include "math/vec3.hpp"
#include "math/bezier.hpp"
#include "rdpq_attach.h"
#include "rdpq_text.h"
#include "types.hpp"
#include "car.hpp"
#include "colors.hpp"

namespace {
  T3DModel *track;
  T3DModel *garage;
  T3DViewport viewport;
  uint8_t color_ambient[4] = {254, 254, 254, 0xFF};
  T3DVec3 light_dir_vec = {{0.f, 1.f, 0.f}};
  color_t light_dir_color {0xFF, 0xFF, 0xFF, 0xFF};
  T3DVec3 cameraPos = {2.92, 346.78, 106.92};
  T3DVec3 origin = {0, 0, 0};
  T3DVec3 yUp = {0.f, 1.f, 0.f};
  jam::Vec3 cameraDistance = {-2.f, -1.f, -2.f};
  int currentBlock = 0;
  color_t currentColors[8];
  T3DQuat rotate;
  T3DMat4FP* garageFP;
  T3DMat4FP* scratchFP[numBlocks];
}
void render_init() {
  // debug font
  rdpq_font_t *fnt1 = rdpq_font_load_builtin(FONT_BUILTIN_DEBUG_MONO);
  rdpq_text_register_font(1, fnt1);

  // t3d
  t3d_init((T3DInitParams){0});

  // models
  track = t3d_model_load("rom://track.t3dm");
  assertf(track != nullptr, "Something went wrong");

  garage = t3d_model_load("rom://garage.t3dm");
  assertf(garage != nullptr, "Something went wrong");

  // viewport, lighting, camera
  viewport = t3d_viewport_create();

  t3d_vec3_norm(&light_dir_vec);
  t3d_quat_identity(rotate);
  garageFP = (T3DMat4FP*)malloc_uncached(sizeof(T3DMat4FP));
  for (int i = 0; i < (int)numBlocks; i++) {
    scratchFP[i] = (T3DMat4FP*)malloc_uncached(sizeof(T3DMat4FP));
    currentColors[i] = COLORS[0];
  }
}

jam::BezierTrack* track_init() {
  int track_data_size = 0;
  // The blender script guarentees we are float point aligned
  float *track_data =
      (float *)asset_load("rom:/track_layout.bin", &track_data_size);
  unsigned short version = 0, num_floats = 0;
  std::vector<jam::Vec3> track_points, normals;

  int header = *(int *)&track_data[0]; // Can be UB
  version = (header & 0xFF000000) >> 24;
  assertf(version == 1, "We expected file type 1");
  int vector_size = (header & 0x00FF0000) >> 16;
  assertf(vector_size == 3,
          "Data file does not contain points of 3 dimensions: %d", vector_size);
  // Load our points
  {
    num_floats = header & 0x0000FFFF;
    num_floats = (num_floats & 0x00FF) | ((num_floats & 0xFF00) >> 8);
    assertf(num_floats == 45, "Float size is wrong");
    track_points.reserve(std::floor(num_floats / vector_size));
    for (size_t i = 0; i < num_floats; i += vector_size) {
      auto p =
          jam::Vec3(track_data[i + 1], track_data[i + 2], -track_data[i + 3]);
      track_points.push_back(std::move(p));
    }
  }
  // Load our normals
  {
    int normals_num_floats = *(int *)&track_data[num_floats + 1];
    normals_num_floats = (normals_num_floats & 0xFFFF0000) >> 16;
    normals_num_floats =
        (normals_num_floats & 0x00FF) | ((normals_num_floats & 0xFF00) >> 8);
    assertf(normals_num_floats == 180, "Normals count incorrect: %d",
            normals_num_floats);
    normals.reserve(normals_num_floats / 12u);
    for (size_t i = 0; i < (size_t)normals_num_floats; i += vector_size) {
      size_t offset = num_floats + i + 2;
      auto p = jam::Vec3(track_data[offset], track_data[offset + 1],
                        -track_data[offset + 2]);
      normals.push_back(std::move(p));
    }
    assertf((normals.size() * vector_size) == (size_t)normals_num_floats,
            "Invalid normals count from blender data %d, expected %d",
            normals.size() * vector_size, normals_num_floats / 4);
  }
  return jam::BezierTrack::from_blender_track_data(
        track_points.size() / 3, track_points, normals);
}

void camera_update(GameStateBook& lastPoints) {
    // Look at our last position, always trailing behind
    // our object, ala unreal spring arm component with a camera
    gamestate_page_t& state = lastPoints.back();
    if (lastPoints.size() < MAX_HISTORY) {
      cameraPos = static_cast<T3DVec3>(state.lastPoint - cameraDistance);
    } else {
      jam::Vec3 lastStop = lastPoints.front().lastPoint;

      auto headingDir = (state.lastPoint - lastStop).normalize();
      auto expectedDistance = std::sqrt(std::pow(cameraDistance.x, 2) + std::pow(cameraDistance.z, 2));
      assertf(expectedDistance > 0.f, "Distance is 0 or negative");
      auto angle = std::atan2(headingDir.z, headingDir.x);
      // assertf(angle > 0.087f, "Angle is < 5deg in radian (0.087)");
      jam::Vec3 p = jam::Vec3(std::cos(angle) * expectedDistance, cameraDistance.y, std::sin(angle) * expectedDistance);
      cameraPos = static_cast<T3DVec3>(state.lastPoint - p);
    }
}

gamestate_page_t& gamestatebook_update(GameStateBook& book, Phase phase) {
  gamestate_page_t newPage {{0,0,0}, 0, 0, phase};
  book.push_back(newPage);
  if (book.size() > MAX_HISTORY)
  {
    book.pop_front(); 
  }
  return book.back();
}

int main(void) {
  // Initialize debug console
  debug_init_isviewer();

  // Setting up the display
  display_init(RESOLUTION_320x240, DEPTH_16_BPP, 3, GAMMA_NONE,
               FILTERS_RESAMPLE_ANTIALIAS_DEDITHER);
  rdpq_init();
  dfs_init(DFS_DEFAULT_LOCATION);
  asset_init_compression(3);
  joypad_init();

  render_init();
  jam::BezierTrack bezierTrack = *track_init();

  // get our initial position for the camera
  Phase currentPhase = Phase::GARAGE;
  GameStateBook gameStateHistory;
  gamestatebook_update(gameStateHistory, currentPhase);
  Car* car = new Car(bezierTrack);


  while (true) {
    joypad_poll();
    auto held = joypad_get_buttons_held(JOYPAD_PORT_1);
    auto pressed = joypad_get_buttons_pressed(JOYPAD_PORT_1);
    int xInput = joypad_get_axis_held(JOYPAD_PORT_1, JOYPAD_AXIS_STICK_X);
    int yInput = joypad_get_axis_held(JOYPAD_PORT_1, JOYPAD_AXIS_STICK_Y);

    // Tick
    gamestate_page_t& gstate = gamestatebook_update(gameStateHistory, currentPhase);
    gstate.lastXInput = xInput;
    gstate.lastYInput = yInput;
    car->Update(gameStateHistory, held);
    currentPhase = gameStateHistory.back().phase;
    if (currentPhase == Phase::RACE) {
      camera_update(gameStateHistory);

      // Render
      t3d_viewport_set_projection(&viewport, T3D_DEG_TO_RAD(65.f), 10.0f,
                                  5000.0f);
      t3d_viewport_look_at(&viewport, &cameraPos, &origin, &yUp);

      rdpq_attach(display_get(), display_get_zbuf());
      t3d_frame_start();
      t3d_viewport_attach(&viewport);

      t3d_screen_clear_color(RGBA32(254, 254, 254, 0xFF));
      t3d_screen_clear_depth();

      t3d_light_set_ambient(color_ambient);
      t3d_light_set_directional(0, &light_dir_color.r, &light_dir_vec);
      t3d_light_set_count(1);

      rdpq_set_prim_color(RGBA32(0xFF, 0xFF, 0xFF, 0xFF));

      T3DModelIter it = t3d_model_iter_create(track, T3D_CHUNK_TYPE_OBJECT);
      while (t3d_model_iter_next(&it)) {
        T3DModelState state = t3d_model_state_create();
        if (strcmp(it.object->name, "Sphere") != 0) {
          t3d_model_draw_material(it.object->material, &state);
          t3d_model_draw_object(it.object, NULL);
        }
      }
      // Setup some colliders for the bezier track points for testing
      for (auto nextStop = bezierTrack.control_points_begin(); nextStop != bezierTrack.control_points_end(); ++nextStop)
      {
        SphereCollider* testCollider = new SphereCollider(*nextStop, 0.20f);
        if (testCollider->Collide(car->Collider()))
        {
          currentPhase = Phase::GARAGE;
        }
      }
      T3DModelState state = t3d_model_state_create();
      car->Render(state);

      rdpq_sync_pipe();
      rdpq_detach_show();
      origin = static_cast<T3DVec3>(gstate.lastPoint);
    } else {
      // Simple camera setup
      T3DVec3 garageCamera {{1777.66357421875, -1.0989729166030884, 12.22758960723877}};
      // Render
      t3d_viewport_set_projection(&viewport, T3D_DEG_TO_RAD(65.f), 10.0f,
                                  5000.0f);
      t3d_viewport_look_at(&viewport, &garageCamera, &origin, &yUp);

      rdpq_attach(display_get(), display_get_zbuf());
      t3d_frame_start();
      t3d_viewport_attach(&viewport);

      t3d_screen_clear_color(RGBA32(200, 200, 200, 0xFF));
      t3d_screen_clear_depth();

      t3d_light_set_ambient(color_ambient);
      t3d_light_set_directional(0, &light_dir_color.r, &light_dir_vec);
      t3d_light_set_count(1);

      t3d_quat_rotate_euler(rotate, (float[3]){0.f, 1.f, 0.f}, -0.1f);
      t3d_mat4fp_from_srt(garageFP, (float[3]){1.f, 1.f, 1.f}, rotate.v, (float[3]){1.f, 1.f, 1.f});
      t3d_matrix_push_pos(1);
      for (int i = 0; i < (int)numBlocks; i++) {
        rdpq_set_prim_color(currentColors[i]);
        jam::Vec3 worldTranslation = translationsWorldSpace[i];
        T3DVec3 t = static_cast<T3DVec3>(worldTranslation);
        T3DQuat rot = (i == currentBlock) ? rotate : (T3DQuat){0.f, 0.f, 0.f, 0.f};  
        t3d_mat4fp_from_srt(scratchFP[i], 
          (float[3]){1.f, 1.f, 1.f}, 
          rot.v,
          t.v);
        t3d_matrix_set(scratchFP[i], true);
        t3d_model_draw(garage);
      }
      t3d_matrix_pop(1); 
      rdpq_sync_pipe();
      rdpq_detach_show();
      if (pressed.start) {
        currentPhase = Phase::RACE;
        car->SetColors(numBlocks, currentColors);
        continue;
      }
      if (pressed.c_left || pressed.c_right) {
        currentBlock += pressed.c_left ? 1 : -1;
      }
      else if (pressed.c_down|| pressed.c_up) {
        currentBlock += pressed.c_down? 1 : -1;
      }
      currentBlock = std::abs(currentBlock) % 8;
      if (pressed.a) {
        if (color_equals(currentColors[currentBlock], RGBA32(0x00, 0x00, 0x00, 0xFF))) {
          currentColors[currentBlock] = COLORS[0];
        }
        else {
          for (int i = 0; i < 4; i++) {
            if (color_equals(currentColors[currentBlock], COLORS[i])) {
              debugf("Set color %d\n", (i + 1) % 4);
              currentColors[currentBlock] = COLORS[(i + 1) % 4];
              break;
            }
          }
        }
      }
    }
  }

  rdpq_close();
  display_close();
  return 0;
}
