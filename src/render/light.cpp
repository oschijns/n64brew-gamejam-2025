#include "render/light.hpp"

using namespace jam;
using namespace jam::render;


void Light::render(uint index)
{
    t3d_light_set_directional(index, color, direction.to_t3d());
}


void Environment::add_light(const Light & light)
{
    lights[light_count] = light;
    ++light_count;
}


void Environment::remove_light(uint index)
{
    --light_count;
    lights[index] = lights[light_count];
}


void Environment::render()
{
    t3d_screen_clear_color(clear);
    t3d_screen_clear_depth();

    t3d_light_set_ambient(ambient);

    // render the lights
    for (uint i = 0; i < light_count; ++i)
        lights[i].render(i);
    t3d_light_set_count(light_count);

    rdpq_set_prim_color(RGBA32(0xFF, 0xFF, 0xFF, 0xFF));
}

