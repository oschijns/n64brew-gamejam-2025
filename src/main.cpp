/*
 * Entry point
 */

#include <libdragon.h>
#include <rdpq.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/gl_integration.h>

// include single header files to test compilation
#if true
#include "math/base.hpp"
#include "math/vec2.hpp"
#include "math/vec3.hpp"
#include "math/quat.hpp"
#include "math/isometry.hpp"
#include "math/jump_trajectory.hpp"
#endif
 
int main(void)
{
    // Initialize debug console
    debug_init_isviewer();

    // Setting up the display and GL
    display_init(RESOLUTION_320x240, DEPTH_16_BPP, 3, GAMMA_NONE, FILTERS_RESAMPLE_ANTIALIAS_DEDITHER);
    rdpq_init();
    gl_init();

    //console_init(); // already calls display_init but without proper parameters
    console_set_render_mode(RENDER_MANUAL);
    console_set_debug(1);

    joypad_init();

    // infinite loop
    while (true) {}

    gl_close();
    rdpq_close();
    display_close();
    return 0;
}