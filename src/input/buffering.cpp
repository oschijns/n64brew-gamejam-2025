#include "input/buffering.hpp"
#include "math/base.hpp"

using namespace jam;
using namespace jam::input;


void Buffering::read_button(joypad_buttons_t btns)
{
    // Check if the button being tracked is being pressed
    pressed = (bool)(btns.raw & (uint) mask);

    // time since system startup
    const uint64_t time = get_ticks();

    // Button is being pressed, check timers
    if (pressed)
    {
        // Were we in a state when the action was allowed to occur?
        if (ticks_delay > (time - timestamp_coyote))
        {
            timestamp_coyote = 0;

            // We were in a state where the action was allowed
            // and we just pressed the button afterwards.
            (action_callback)();
        }
    }
    else
        timestamp_buffer = time;
}


void Buffering::set_new_state(bool new_state)
{
    if (new_state != allow_action)
    {
        // time since system startup
        const uint64_t time = get_ticks();

        // We are leaving the state allowing the action to occur.
        if (not new_state)
            timestamp_coyote = time;

        // We are entering the state allowing the action to occur, 
        // check if we pressed the button just earlier.
        else if (ticks_delay > (time - timestamp_buffer))
        {
            timestamp_buffer = 0;

            // The button was pressed and we are in a
            // state where we can perform the action.
            (action_callback)();
        }

        // update the allow state flag
        allow_action = new_state;
    }
}