/*
 * Entry point
 */

#include <libdragon.h>
#include <rdpq.h>
#include <t3d/t3d.h>
#include <t3d/t3dmodel.h>
//#include <GL/gl.h>
//#include <GL/glu.h>
//#include <GL/gl_integration.h>

#include "track/model.hpp"
#include "render/camera.hpp"
#include "render/light.hpp"

#include <vector>

// include single header files to test compilation
#if true
#include "math/base.hpp"
#include "math/vec2.hpp"
#include "math/vec3.hpp"
#include "math/quat.hpp"
#include "math/isometry.hpp"
#include "math/jump_trajectory.hpp"
#endif
 
namespace jam
{
    /// @brief Current track being loaded
    static track::Model track_curve;

    /// @brief 3D mesh to render
    static T3DModel * track_model = nullptr;

    /// @brief Camera
    static render::Camera camera;

    /// @brief Environment
    static render::Environment environment;

    /// @brief Setup the Game
    void initialize()
    {
        // Initialize debug console
        debug_init_isviewer();

        // Setting up the display and GL
        display_init(RESOLUTION_320x240, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_RESAMPLE_ANTIALIAS_DEDITHER);
        rdpq_init();    // 2D rendering
        //gl_init();    // OpenGL

        // Tiny3D
        const T3DInitParams params = {.matrixStackSize = 8};
        t3d_init(params);

        dfs_init(DFS_DEFAULT_LOCATION);
        asset_init_compression(3);
        joypad_init();

        // debug font
        rdpq_font_t *const fnt1 = rdpq_font_load_builtin(FONT_BUILTIN_DEBUG_MONO);
        rdpq_text_register_font(1, fnt1);

        //console_init(); // already calls display_init but without proper parameters
        console_set_render_mode(RENDER_MANUAL);
        console_set_debug(1);
    }


    /// @brief Load a given track to test it
    void load_track()
    {
        // Curve model
        track::Model curve;
        track::LoadError error = track::Model::load_from_file("rom://sample.bin", curve);
        assertf(error == track::LoadError::OK, "Failed to load curve track");

        // Tiny3D models
        T3DModel *const track = t3d_model_load("rom://track.t3dm");
        assertf(track != nullptr, "Failed to load Tiny3D model");
    }

    /// @brief Setup viewport, lighting and camera
    void setup_rendering()
    {
        // Pulled these from blender.
        // If you want the relevant python
        // `mathutils.Matrix.decompose(C.scene.camera.matrix_world)[0].xzy * 64`
        // T3DVec3 cameraPos = {312.9292297363281, 186.78846740722656, 381.92987060546875};
        camera.origin = Vec3(2.92, 346.78, 106.92);

        // Add a light to the environment
        environment.add_light(render::Light());
    }

    /// @brief Called repeatedly in a loop
    void update()
    {
        // read inputs
        joypad_poll();
        joypad_buttons_t btns = joypad_get_buttons_pressed(JOYPAD_PORT_1);

        camera.update_projection();
        camera.update_look_at();

        rdpq_attach(display_get(), display_get_zbuf());
        t3d_frame_start();
        camera.attach_viewport();
        environment.render();

        rdpq_sync_pipe();
        rdpq_text_printf(NULL, 1, 30, 60, "1 - Teal\n2 - Green\n3 - Blue\n4 - Purple");
        rdpq_detach_show();
    }


    /// @brief Terminate the Game
    /// @note This should never be called actually
    void terminate()
    {
        t3d_destroy();  // Tiny3D
        //gl_close();   // OpenGL
        rdpq_close();   // 2D rendering
        display_close();
    }
}


int main(void)
{
    jam::initialize();
    jam::load_track();
    jam::setup_rendering();

    // infinite loop
    while (true)
    {
        jam::update();
    }

    jam::terminate();
    return 0;
}